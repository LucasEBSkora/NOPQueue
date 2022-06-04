#include <iostream>
#include <queue>
#include <vector>
#include <functional>

typedef std::queue<std::function<void(void)>> NOPQueue;

NOPQueue NotificationQueue;


struct Main {

  struct {bool old; bool current;} atStatus;
  struct {bool old; bool current;} prStatus;
  struct {bool old; bool current;} cdStatus;

  void mtChange() {
    std::cout << "hello world!" << std::endl;
    UpdateAtStatus(true);
  }

  void NotifyinChange() {
    NotificationQueue.push([&, this] {this->mtChange();});
  }

  void NotifyAcChange() {
    NotificationQueue.push([&, this] {this->NotifyinChange();});
  }

  void NotifyRlChange() {
    NotificationQueue.push([&, this] {this->NotifyAcChange();});
  }

  void UpdateCdStatus() {
    cdStatus.old = cdStatus.current;
    cdStatus.current = prStatus.current;
    if (cdStatus.current != cdStatus.old) {
      NotificationQueue.push([&, this] {this->NotifyCdStatus();});    
    } 
  }

  void NotifyCdStatus() {
    if (cdStatus.current) {
      NotificationQueue.push([&, this] {this->NotifyRlChange();});    
    }
  }


  void UpdatePrStatus() {
    prStatus.old = prStatus.current;
    prStatus.current = (atStatus.current == true);
    if (prStatus.current != prStatus.old) {
      NotificationQueue.push([&, this] {this->NotifyPrStatus();});    
    }
  }

  void NotifyPrStatus() {
      UpdateCdStatus();
  }

  void UpdateAtStatus(bool newValue) {
    atStatus.old = atStatus.current;
    atStatus.current = newValue;
    if (atStatus.current != atStatus.old) {
      NotificationQueue.push([&, this] {this->NotifyAtStatus();});
    }
  }

  void NotifyAtStatus() {
      UpdatePrStatus();
  }

  Main() : atStatus{false, false}, prStatus{false, false}, cdStatus{false, false} {
    UpdateAtStatus(true);
    
  }

};

int main()
{
  Main main;
  while (!NotificationQueue.empty()) {
    NotificationQueue.front()();
    NotificationQueue.pop();
  }
  
  return 0;
}
