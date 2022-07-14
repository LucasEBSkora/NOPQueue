#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <variant>
#include <fstream>

std::ofstream logFile;

using PrimitiveType = std::variant<long long, double, bool, std::string>;
using NotificationValue = std::variant<long long, double, bool, std::string, std::vector<PrimitiveType>>;

typedef struct
{
  NotificationValue value;
  bool nonotify;
  bool renotify;
} NotificationParameters;

using Timestamp = unsigned char;

typedef struct vertex_s
{
  NotificationValue state;
  unsigned char updateFunction;
  unsigned char decidePropagateFunction;
  unsigned char newParametersFunction;
  unsigned char sideEffect;

  unsigned int headOfAtomicGroup;

  unsigned int lengthLongestPath;
  unsigned int depth;

  unsigned char pendingDependents;
  Timestamp lastEnqueued;
  Timestamp nextNotify;
  bool notificationPending;
  ~vertex_s() {}
} Vertex;

typedef struct edge_s
{
  unsigned int source;
  unsigned int destination;
  bool dependency;
  unsigned int atomicGroupID;
} Edge;

typedef struct pendingNotification_s
{
  unsigned int v;
  NotificationParameters p;
  Edge e;
  unsigned int H;
  Timestamp j;
} PendingNotification;

using NotificationQueue = std::queue<PendingNotification>;

NotificationQueue notificationQueue;

typedef struct ve_s
{
  Vertex v;
  std::vector<Edge> adjacency;
  std::vector<unsigned int> reverseDependencies;
  ~ve_s() {}
} VE;

const size_t size = 6;
VE ENGraph[] = {
    {{false, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0}, {{0, 1, true, 0}}, {}},  // attribute
    {{false, 2, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0}, {{1, 2, true, 0}}, {0}}, // premise
    {{false, 3, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0}, {{2, 3, true, 0}}, {1}}, // condition
    {{false, 4, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0}, {{3, 4, true, 0}}, {2}}, // rule
    {{false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {{4, 5, false, 0}}, {}}, // action
    {{false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {{5, 6, false, 0}}, {}}, // instigation
    {{false, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0}, {{6, 7, false, 2}}, {}}, // method
    {{false, 0, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0}, {}, {}}                  // callExternal
};

NotificationValue updateValue(Vertex *v, NotificationParameters p)
{
  switch (v->updateFunction)
  {
  case 0: // no state
    return false;
  case 1: // from parameter
    return p.value;
  // from expression
  case 2:
    return ENGraph[0].v.state;
  case 3:
    return ENGraph[1].v.state;
  case 4:
    return ENGraph[2].v.state;
  // not registered
  default:
    return false;
  }
}

bool decidePropagate(Vertex *v, NotificationParameters p, NotificationValue nv)
{
  if (p.nonotify)
  {
    return false;
  }
  switch (v->decidePropagateFunction)
  {
  case 0: // always
    return true;
  case 1: // if state changed
    return v->state != nv || p.renotify;
  case 2: // if state changed to true
    return std::get<bool>(nv) && (v->state != nv || p.renotify);
  // not registered
  default:
    return false;
  }
}

void sideEffect(Vertex *v)
{
  switch (v->sideEffect)
  {
  case 0: // nothing
    return;
  case 1:
    std::cout << "Hello World!" << std::endl;
    return;
  default:
    return;
  }
}

NotificationParameters newParameters(Vertex *v, NotificationParameters old_p)
{
  switch (v->newParametersFunction)
  {
  case 0: // no change
    return old_p;
  default:
    return old_p;
  }
}

unsigned int atomicGroupSizes[] = {
    0, // No group
    1, // Attribute
    1  // Method
};

bool edgeInAtomicGroup(unsigned int edgeAtomicGroup, unsigned int atomicGroup)
{
  return atomicGroup <= edgeAtomicGroup && edgeAtomicGroup < atomicGroup + atomicGroupSizes[atomicGroup];
}

bool execute()
{
  while (!notificationQueue.empty())
  {
    PendingNotification t = notificationQueue.front();
    notificationQueue.pop();
    Vertex *v = &ENGraph[t.v].v;
    logFile << "popped (" << t.v
              << ", {" << std::get<bool>(t.p.value) << (t.p.nonotify ? ", nonotify" : "") << (t.p.renotify ? ", renotify" : "") << "}, "
              << "(" << t.e.source << ", " << t.e.destination << ")" << (t.e.atomicGroupID != 0 ? " in D" : "") << (t.e.dependency ? " in D" : "")
              << ", " << t.H << ", " << int(t.j) << ")\n";

    if (!notificationQueue.empty())
    {
      if ((v->headOfAtomicGroup != 0 && t.j != v->nextNotify) || v->pendingDependents > 0)
      {
        logFile << "\tnotification to head of atomic group " << v->headOfAtomicGroup << " needs to be delayed: ";
        if (t.j != v->nextNotify)
        {
          logFile << int(t.j) << " != " << int(v->nextNotify);
        }
        else
        {
          logFile << "there are " << int(v->pendingDependents) << "pending notifications to vertices depending on " << t.v;
        }
        logFile << "\n";

        notificationQueue.push(t);
        continue;
      }
      if (v->depth < v->lengthLongestPath)
      {
        logFile << "\tnotification to vertex " << t.v << " needs to be delayed: current depth needs to be "
                  << v->lengthLongestPath << " and is currently " << v->depth << "\n";
        ++(v->depth);
        notificationQueue.push(t);
        continue;
      }
    }

    NotificationValue newState = updateValue(v, t.p);
    bool propagate = decidePropagate(v, t.p, newState);
    logFile << "\tnotifying " << t.v << ": old value is " << std::get<bool>(v->state) << ", new value is " << std::get<bool>(newState);
    v->state = newState;
    sideEffect(v);
    if (propagate)
    {
      NotificationParameters p = newParameters(v, t.p);
      logFile << " decided to propagate with new parameters {"
                << std::get<bool>(p.value) << (p.nonotify ? ", nonotify" : "") << (p.renotify ? ", renotify" : "") << "}\n";
      std::vector<Edge> &adjacency = ENGraph[t.v].adjacency;
      for (Edge e : adjacency)
      {
        Vertex &u = ENGraph[e.destination].v;

        if (u.headOfAtomicGroup != 0)
        {
          ++u.lastEnqueued;
          if (edgeInAtomicGroup(e.atomicGroupID, t.H))
          {
            notificationQueue.push({e.destination, p, e, t.H, u.lastEnqueued});

            logFile << "\t\tenqueuing notification to head in same atomic group (" << e.destination
                      << ", {" << std::get<bool>(p.value) << (p.nonotify ? ", nonotify" : "") << (p.renotify ? ", renotify" : "") << "}, "
                      << "(" << e.source << ", " << e.destination << ")" << (e.atomicGroupID != 0 ? " in D" : "") << (e.dependency ? " in D" : "")
                      << ", " << t.H << ", " << int(u.lastEnqueued) << ")\n";
          }
          else
          {
            notificationQueue.push({e.destination, p, e, u.headOfAtomicGroup, u.lastEnqueued});

            logFile << "\t\tenqueuing notification to head's atomic group (" << e.destination
                      << ", {" << std::get<bool>(p.value) << (p.nonotify ? ", nonotify" : "") << (p.renotify ? ", renotify" : "") << "}, "
                      << "(" << e.source << ", " << e.destination << ")" << (e.atomicGroupID != 0 ? " in D" : "") << (e.dependency ? " in D" : "")
                      << ", " << u.headOfAtomicGroup << ", " << int(u.lastEnqueued) << ")\n";
          }
        }
        else
        {
          if (!e.dependency)
          {
            notificationQueue.push({e.destination, p, e, t.H, 0});

            logFile << "\t\tenqueuing notification  (" << e.destination
                      << ", {" << std::get<bool>(p.value) << (p.nonotify ? ", nonotify" : "") << (p.renotify ? ", renotify" : "") << "}, "
                      << "(" << e.source << ", " << e.destination << ")" << (e.atomicGroupID != 0 ? " in D" : "") << (e.dependency ? " in D" : "")
                      << ", " << t.H << ", 0)\n";
          }
          else
          {
            ++(v->pendingDependents);
            if (!u.notificationPending)
            {
              u.notificationPending = true;
              notificationQueue.push({e.destination, p, e, t.H, 0});
              logFile << "\t\tenqueuing notification  (" << e.destination
                        << ", {" << std::get<bool>(p.value) << (p.nonotify ? ", nonotify" : "") << (p.renotify ? ", renotify" : "") << "}, "
                        << "(" << e.source << ", " << e.destination << ")" << (e.atomicGroupID != 0 ? " in D" : "") << (e.dependency ? " in D" : "")
                        << ", " << t.H << ", 0)\n";
            }
            else
            {
              logFile << "(" << e.source << ", " << e.destination << ")"
                        << (e.atomicGroupID != 0 ? " in D" : "") << (e.dependency ? " in D" : "")
                        << ": adjusting depth of next notification to " << e.destination << "to " << v->depth + 1 << '\n';
            }
            u.depth = v->depth + 1;
          }
        }
      }
    }
    else
    {
      logFile << '\n';
    }
    if (v->headOfAtomicGroup != 0)
    {
      ++(v->nextNotify);
    }
    if (t.e.dependency)
    {
      v->notificationPending = false;
    }
    for (unsigned int i : ENGraph[t.v].reverseDependencies)
    {
      --ENGraph[i].v.pendingDependents;
    }
  }
  return true;
}

bool beginNotify(unsigned int v, NotificationParameters parameters)
{
  unsigned int H = ENGraph[v].v.headOfAtomicGroup;
  if (H == 0)
  {
    return false;
  }

  for (size_t i = 0; i < size; ++i)
  {
    ENGraph[i].v.depth = 0;
    ENGraph[i].v.pendingDependents = 0;
    ENGraph[i].v.lastEnqueued = 0;
    ENGraph[i].v.nextNotify = 0;
    ENGraph[i].v.notificationPending = false;
  }
  ENGraph[v].v.lastEnqueued = 1;
  notificationQueue.push({v, parameters, {0, v, false, 0}, H, 0});

  return execute();
}

int main()
{
  logFile.open("log.txt");
  beginNotify(0, {true, false, false});
  logFile.close();
  return 0;
}
