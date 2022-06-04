#include <iostream>
#include <deque>
#include <vector>
#include <functional>

#define LAMBDA_CALL(f) ([this] { f ();})
#define LAMBDA_EXPRESSION(e) ([this] {return e;})
typedef std::deque<std::function<void(void)>> NOPQueue;

NOPQueue NotificationQueue;

struct BooleanAttribute {
  bool value;
  const std::vector<std::function<void(void)>> notifyList;

  BooleanAttribute(bool value, const std::vector<std::function<void(void)>> notifyList) : value{value}, notifyList{notifyList} {}

    void update(bool newValue) {
    if (value != newValue) {
      NotificationQueue.push_back([&, this] {this->notify();});
      value = newValue;
    }
  }
  
  void notify() {
      NotificationQueue.insert(NotificationQueue.end(), notifyList.begin(), notifyList.end());
  }
};

struct Premise {
  std::function<bool(void)> expression;
  bool value;
  const std::vector<std::function<void(void)>> notifyList;

  Premise(std::function<bool(void)> expression, const std::vector<std::function<void(void)>> notifyList) 
    : expression{expression}, value{expression()}, notifyList{notifyList} {}

  void update() {
    bool newValue = expression();
    if (value != newValue) {
      NotificationQueue.push_back([&, this] {this->notify();});
      value = newValue;
    }
  }
  
  void notify() {
      NotificationQueue.insert(NotificationQueue.end(), notifyList.begin(), notifyList.end());
  }
};

struct Condition {
  std::function<bool(void)> expression;
  bool value;
  const std::vector<std::function<void(void)>> notifyList;

  Condition(std::function<bool(void)> expression, const std::vector<std::function<void(void)>> notifyList) 
    : expression{expression}, value{expression()}, notifyList{notifyList} {}

  void update() {
    bool newValue = expression();
    if (value != newValue) {
      NotificationQueue.push_back([&, this] {this->notify();});
      value = newValue;
    }
  }
  
  void notify() {
    if (value) {
      NotificationQueue.insert(NotificationQueue.end(), notifyList.begin(), notifyList.end());
    }
  }
};



struct Main {

  BooleanAttribute atStatus;
  Premise prStatus;
  Condition cdStatus;

  void mtChange() {
    std::cout << "hello world!" << std::endl;
    atStatus.update(true);
  }

  void NotifyinChange() {
    NotificationQueue.push_back([&, this] {this->mtChange();});
  }

  void NotifyAcChange() {
    NotificationQueue.push_back([&, this] {this->NotifyinChange();});
  }

  void NotifyRlChange() {
    NotificationQueue.push_back([&, this] {this->NotifyAcChange();});
  }

  Main() : 
    atStatus{false, {LAMBDA_CALL(prStatus.update)}}, 
    prStatus{LAMBDA_EXPRESSION(atStatus.value == true), {LAMBDA_CALL(cdStatus.update)}}, 
    cdStatus{LAMBDA_EXPRESSION(prStatus.value), {LAMBDA_CALL(NotifyRlChange)}} {
    atStatus.update(true);
  }

};

int main()
{
  Main main;
  while (!NotificationQueue.empty()) {
    NotificationQueue.front()();
    NotificationQueue.pop_front();
  }
  
  return 0;
}
