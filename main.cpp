#include <iostream>
#include <queue>
#include <vector>
#include <functional>

typedef std::queue<std::function<void(void)>> NOPQueue;

NOPQueue NotificationQueue;


struct Main {

  bool atStatus;
  bool prStatus;
  bool cdStatus;

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
    bool newValue = prStatus;
    if (cdStatus != newValue) {
      NotificationQueue.push([&, this] {this->NotifyCdStatus();});    
      cdStatus = newValue;
    } 
  }

  void NotifyCdStatus() {
    if (cdStatus) {
      NotificationQueue.push([&, this] {this->NotifyRlChange();});    
    }
  }


  void UpdatePrStatus() {
    bool newValue = (atStatus == true);
    if (prStatus != newValue) {
      NotificationQueue.push([&, this] {this->NotifyPrStatus();});    
      prStatus = newValue;
    }
  }

  void NotifyPrStatus() {
      UpdateCdStatus();
  }

  void UpdateAtStatus(bool newValue) {
    if (atStatus != newValue) {
      NotificationQueue.push([&, this] {this->NotifyAtStatus();});
    atStatus = newValue;
    }
  }

  void NotifyAtStatus() {
      UpdatePrStatus();
  }

  Main() : atStatus{false}, prStatus{false}, cdStatus{false} {
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
