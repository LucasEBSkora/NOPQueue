#include <iostream>
#include <deque>
#include <vector>
#include <functional>

#define LAMBDA_CALL(f) ([this] { f(); })
#define LAMBDA_EXPRESSION(e) ([this] { return e; })
typedef std::deque<std::function<void(void)>> NOPQueue;

NOPQueue NotificationQueue;

template <typename T>
class Attribute
{
  T value;
  const std::vector<std::function<void(void)>> notifyList;

public:
  Attribute(T value, const std::vector<std::function<void(void)>> notifyList) : value{value}, notifyList{notifyList} {}

  T operator = (T newValue) {
    if (value != newValue)
    {
      NotificationQueue.push_back([&, this]
                                  { this->notify(); });
      value = newValue;
    }
    return newValue;
  }

  void notify()
  {
    NotificationQueue.insert(NotificationQueue.end(), notifyList.begin(), notifyList.end());
  }

  operator T()
  {
    return value;
  }
};

class Premise
{

  std::function<bool(void)> expression;
  bool value;
  const std::vector<std::function<void(void)>> notifyList;

public:
  Premise(std::function<bool(void)> expression, const std::vector<std::function<void(void)>> notifyList)
      : expression{expression}, value{expression()}, notifyList{notifyList} {}

  void update()
  {
    bool newValue = expression();
    if (value != newValue)
    {
      NotificationQueue.push_back([&, this]
                                  { this->notify(); });
      value = newValue;
    }
  }

  void notify()
  {
    NotificationQueue.insert(NotificationQueue.end(), notifyList.begin(), notifyList.end());
  }

  inline operator bool()
  {
    return value;
  }
};

class Condition
{
  std::function<bool(void)> expression;
  bool value;
  const std::vector<std::function<void(void)>> notifyList;

public:
  Condition(std::function<bool(void)> expression, const std::vector<std::function<void(void)>> notifyList)
      : expression{expression}, value{expression()}, notifyList{notifyList} {}

  void update()
  {
    bool newValue = expression();
    if (value != newValue)
    {
      NotificationQueue.push_back([&, this]
                                  { this->notify(); });
      value = newValue;
    }
  }

  void notify()
  {
    if (value)
    {
      NotificationQueue.insert(NotificationQueue.end(), notifyList.begin(), notifyList.end());
    }
  }

  inline operator bool()
  {
    return value;
  }
};

struct Main
{

  Attribute<bool> atStatus;
  Premise prStatus;
  Condition cdStatus;

  void mtChange()
  {
    std::cout << "hello world!" << std::endl;
    atStatus = true;
  }

  void NotifyinChange()
  {
    NotificationQueue.push_back([&, this]
                                { this->mtChange(); });
  }

  void NotifyAcChange()
  {
    NotificationQueue.push_back([&, this]
                                { this->NotifyinChange(); });
  }

  void NotifyRlChange()
  {
    NotificationQueue.push_back([&, this]
                                { this->NotifyAcChange(); });
  }

  Main() : atStatus{false, {LAMBDA_CALL(prStatus.update)}},
           prStatus{LAMBDA_EXPRESSION(atStatus == true), {LAMBDA_CALL(cdStatus.update)}},
           cdStatus{LAMBDA_EXPRESSION(prStatus), {LAMBDA_CALL(NotifyRlChange)}}
  {
    atStatus = true;
  }
};

int main()
{
  Main main;
  while (!NotificationQueue.empty())
  {
    NotificationQueue.front()();
    NotificationQueue.pop_front();
  }

  return 0;
}
