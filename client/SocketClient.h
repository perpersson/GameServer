#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

class SocketClient
{
 public:
  SocketClient(const char* hostname, int port);

  void mainLoop();

 protected:
  int sock;

  // Methods to override in specific game clients subclasses.
  virtual void onServerConnected() {}
  virtual void onWaitForInput() {}
  virtual void onStdinDataReceived(char* data, unsigned int dataLength) = 0;
  virtual void onServerDataReceived(char* data, unsigned int dataLength) = 0;

 private:
  void connectToServer(const char* hostname, int port);
  
  void waitForInput(fd_set* read_fds, bool waitStdin,
                    bool waitSocket, int timeoutInMilliSeconds = 0);

  bool checkForStdinData(fd_set* read_fds);
  bool checkForSocketData(fd_set* read_fds);

  const char* hostname;
  int port;
};

#endif
