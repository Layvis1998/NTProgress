#include <cstdlib>
#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <unordered_map>
#include "json.hpp"
#include "Common.hpp"
#include <stdint.h>
#include <list>
#include <vector>

using boost::asio::ip::tcp;
using std::string;
using std::unordered_map;
using std::to_string;
using std::list;
using std::vector;

struct application
{
  int id;
  int amount;
  int price;
};

struct L_application
{
  int id;
  int amount;
  int price;
  bool type; //true <-> buy, false <-> sell

  L_application (int a, int b, int c, bool d)
  {
    id = a;
    amount = b;
    price = c;
    type = d;
  } 
};

struct user
{
  string name;
  string password;
  int rubles;
  int dollars;
  list<L_application> deals;
};

unordered_map<size_t, user> People;

class DOM
{
public:
  list <application> buy_list;
  list <application> sell_list;

  string UserDeals(const std::string& ID)
  {
    string retval = "";
    
    auto it_begin = People[std::stoi(ID)].deals.begin();
    auto it_end = People[std::stoi(ID)].deals.end();

    for (auto it = it_begin; it != it_end; it++)
    {
      if (it->type == 1)
        retval += " seller id: ";
      else
        retval += " buyer id: ";
      retval = retval + to_string(it->id) + ", "; 
      retval = retval + "amount: " + to_string(it->amount) + ", ";
      retval = retval + "price: " + to_string(it->price) + "\n";
    }

    retval = retval + "\n";
  
    if (retval == "")
      return "You deal archive is still empty\n";
    else
      return retval;
  }


  void CreateBuyApp (int id, int amnt, int pr)
  {
    auto it = buy_list.begin();
    while ((it != buy_list.end()) && (it->price >= pr)) 
      it++;
  
    buy_list.insert(it, {id, amnt, pr});
  }
  
  void CreateSellApp (int id, int amnt, int pr)
  {
    auto it = sell_list.begin();
    while ((it != sell_list.end()) && (it->price <= pr))
      it++;
    
    sell_list.insert(it, {id, amnt, pr});
  }

  string CancellApp(int id, int amnt, int pr, string type)
  {
    if (type == "+")
    {
      auto it = buy_list.begin();
      while (it != buy_list.end())
      {
        if ((id == it->id) && (pr == it->price))
        {
          if (it->amount > amnt)
          {
            it->amount -= amnt;
          }
          else
          {
            it = buy_list.erase(it);
          }
          return "App successfully cancelled\n";
        }
        it++;
      }
      return "App not found\n"; 
    }
    else if (type == "-")
    {
      auto it = sell_list.begin();
      while (it != sell_list.end())
      {
        if ((id == it->id) && (pr == it->price))
        {
          if (it->amount > amnt)
          {
            it->amount -= amnt;
          }
          else
          {
            it = buy_list.erase(it);
          }
          return "App successfully cancelled\n";
        }
        it++;
      }
      return "App not found\n"; 
    }
    return "App not found\n"; 
  }

  string Display_apps()
  {
    string retval = "";
       
    retval = "buy apps (rub per $, amount):\n";
    for (auto it = buy_list.begin(); it != buy_list.end();
      it++)
    {
      retval = retval + to_string(it->price)  + " "
                      + to_string(it->amount) + ", "; 
    }
    retval += "\n///////////////////////////////////////////////\n";

    retval += "sell apps (rub per $, amount):\n";
    for (auto it = sell_list.begin(); it != sell_list.end(); it++)
    {
      retval = retval + to_string(it->price) + " "
                      + to_string(it->amount) + ", "; 
    }
    retval += "\n";
    return retval;
  }

  string ShowUserApps(int id)
  {
    string retval = "";
       
    retval = "Your buy apps (rub per $, amount):\n";
    for (auto it = buy_list.begin(); it != buy_list.end(); it++)
    {
      if (it->id == id)
      {    
        retval = retval + to_string(it->price)  + " " 
                        + to_string(it->amount) + ", "; 
      }
    }
    retval += "\n///////////////////////////////////////////////\n";

    retval += "Your sell apps (rub per $, amount):\n";
    for (auto it = sell_list.begin(); it != sell_list.end(); it++)
    { 
      if (it->id == id)
      {      
        retval = retval + to_string(it->price) + " "
                        + to_string(it->amount) + ", "; 
      }
    }
    retval += "\n";
    return retval;
  }
  
  void CheckBargain(int id)
  {
    list<application>::iterator buy_it = buy_list.begin();
    list<application>::iterator sell_it = sell_list.begin();

    while ((buy_it != buy_list.end())
       && (sell_it != sell_list.end())
       && (buy_it->price >= sell_it->price))
    {
      if (sell_it->amount > buy_it->amount)
      {
        if (id == sell_it->id)
        {
          People[id].deals.emplace_back(buy_it->id, buy_it->amount,
            buy_it->price, false);
          People[buy_it->id].deals.emplace_back(id,
            buy_it->amount, buy_it->price, true);
        }
        else
        {
          People[id].deals.emplace_back(sell_it->id, buy_it->amount,
            buy_it->price, true);
          People[sell_it->id].deals.emplace_back(buy_it->id,
            buy_it->amount, buy_it->price, false);
        }

        sell_it->amount -= buy_it->amount;
        People[sell_it->id].rubles += buy_it->amount * buy_it->price;
        People[sell_it->id].dollars -= buy_it->amount;
        People[buy_it->id].rubles -= buy_it->amount * buy_it->price;
        People[buy_it->id].dollars += buy_it->amount;    
        buy_it = buy_list.erase(buy_it);
      }
      else if (sell_it->amount < buy_it->amount)
      {
        if (id == sell_it->id)
        {
          People[id].deals.emplace_back(buy_it->id, sell_it->amount,
            buy_it->price, false);
          People[buy_it->id].deals.emplace_back(id,
            sell_it->amount, buy_it->price, true);
        }
        else
        {
          People[id].deals.emplace_back(sell_it->id, sell_it->amount,
            buy_it->price, true);
          People[sell_it->id].deals.emplace_back(buy_it->id,
            sell_it->amount, buy_it->price, false);
        }

        buy_it->amount -= sell_it->amount;
        People[sell_it->id].rubles += sell_it->amount * buy_it->price;
        People[sell_it->id].dollars -= sell_it->amount;
        People[buy_it->id].rubles -= sell_it->amount * buy_it->price;
        People[buy_it->id].dollars += sell_it->amount;  
        sell_it = sell_list.erase(sell_it);;
      }
      else if (sell_it->amount == buy_it->amount)
      {
        if (id == sell_it->id)
        {
          People[id].deals.emplace_back(buy_it->id, buy_it->amount,
            buy_it->price, false);
          People[buy_it->id].deals.emplace_back(id,
            sell_it->amount, buy_it->price, true);
        }
        else
        {
          People[id].deals.emplace_back(sell_it->id, buy_it->amount,
            buy_it->price, true);
          People[sell_it->id].deals.emplace_back(buy_it->id,
            sell_it->amount, buy_it->price, false);
        }

        People[sell_it->id].rubles += buy_it->amount * buy_it->price;
        People[sell_it->id].dollars -= buy_it->amount;
        People[buy_it->id].rubles -= buy_it->amount * buy_it->price;
        People[buy_it->id].dollars += buy_it->amount; 
        buy_it = buy_list.erase(buy_it); 
        sell_it = sell_list.erase(sell_it);
      }

    }
  }
};

DOM dom1;

class Core
{
public:
  // "Регистрирует" нового пользователя и возвращает его ID.
  string RegisterNewUser(const string& aUserName)
  {
    int newUserId = People.size();
    People[newUserId].name = aUserName;
    return to_string(newUserId);
  }

  string CreatePassword(const string& ID, const string& pswrd)
  {
    People[std::stoi(ID)].password = pswrd;
    return pswrd;
  }

  string CheckPassword(const string& ID, const string& pswrd)
  {
    if (People[std::stoi(ID)].password == pswrd)
      return "correct";
    else
      return "incorrect";
  }

  string CheckID(const string& ID)
  {
    if (People.count(std::stoi(ID)))
      return "correct";
    else
      return "incorrect";
  }

  // Запрос имени клиента по ID
  string GetUserName(const std::string& aUserId)
  {
    return People[std::stoi(aUserId)].name;
  }

  // Запрос на баланс по ID
  string GetBalance(const std::string& aUserId)
  {
    return to_string(People[std::stoi(aUserId)].rubles) + " rub, "
      + to_string(People[std::stoi(aUserId)].dollars) + " dollars";
  }
};

Core& GetCore()
{
  static Core core;
  return core;
}

class session
{
private:
  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];

public:
  session(boost::asio::io_service& io_service)
        : socket_(io_service) {}

  tcp::socket& socket()
  {
    return socket_;
  }

  void start()
  {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
    boost::bind(&session::handle_read, this,
    boost::asio::placeholders::error,
    boost::asio::placeholders::bytes_transferred));
  }

  // Обработка полученного сообщения.
  void handle_read(const boost::system::error_code& error,
    size_t bytes_transferred)
  {
    if (!error)
    {
      data_[bytes_transferred] = '\0';

      // Парсим json, который пришёл нам в сообщении.
      auto j = nlohmann::json::parse(data_);
      auto reqType = j["ReqType"];

      string reply = "Error! Unknown request type";
      if (reqType == Requests::Registration)
      {
        // Это реквест на регистрацию пользователя.
        // Добавляем нового пользователя и возвращаем его ID.
        reply = GetCore().RegisterNewUser(j["Message"]);
      }
      else if (reqType == Requests::Hello)
      {
        reply = "Your user name is " + GetCore().GetUserName(j["UserId"]) + "\n";
      }
      else if (reqType == Requests::CreatePassword)
      {
        reply = GetCore().CreatePassword(j["UserId"], j["Message"]);
      }
      else if (reqType == Requests::CheckID)
      {
        reply = GetCore().CheckID(j["UserId"]);
      }
      else if (reqType == Requests::CheckPassword)
      {
        reply = GetCore().CheckPassword(j["UserId"], j["Message"]);
      }
      else if (reqType == Requests::Balance)
      {
        reply = "Your balance is: " + GetCore().GetBalance(j["UserId"]) + "\n";
      }
      else if (reqType == Requests::BuyApp)
      {
        string str = j["Message"];
        std::istringstream iss(str);
        float amnt = 0;
        float pr = 0;
        iss >> amnt >> pr;
        str = j["UserId"];
        int id = stoi(str);
        dom1.CreateBuyApp(id, floor(amnt), floor(pr));

        reply = "\n";
        dom1.CheckBargain(id);
      }
      else if (reqType == Requests::SellApp)
      {
        string str = j["Message"];
        std::istringstream iss(str);
        float amnt = 0;
        float pr = 0;
        iss >> amnt >> pr;
        str = j["UserId"];
        int id = stoi(str);

        dom1.CreateSellApp(id, floor(amnt), floor(pr));

        reply = "\n";
        dom1.CheckBargain(id);
      }
      else if (reqType == Requests::CancelApp)
      {
        string str = j["Message"];
        std::istringstream iss(str);
        float amnt = 0;
        float pr = 0;
        string type;
        iss >> amnt >> pr >> type;
        str = j["UserId"];
        int id = stoi(str);

        reply = dom1.CancellApp(id, floor(amnt), floor(pr), type);
      }
      else if (reqType == Requests::ActiveApps)
      {
        string str = j["UserId"];
        int id = stoi(str);
        reply = dom1.ShowUserApps(id);
      }
      else if (reqType == Requests::ShowDOM)
      {
        reply = dom1.Display_apps();
      }
      else if (reqType == Requests::MyDeals)
      {
        string id = j["UserId"];
        reply = dom1.UserDeals(id);
      }

      boost::asio::async_write(socket_,
        boost::asio::buffer(reply, reply.size()),
        boost::bind(&session::handle_write, this,
        boost::asio::placeholders::error));
    }
    else
    {
      delete this;
    }
  }

  void handle_write(const boost::system::error_code& error)
  {
    if (!error)
    {
      socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      delete this;
    }
  }

};

class server
{
private:
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;

public:
  server(boost::asio::io_service& io_service)
    : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
   {
     std::cout << "Server started! Listen " << port << " port" << std::endl;

     session* new_session = new session(io_service_);
     acceptor_.async_accept(new_session->socket(),
       boost::bind(&server::handle_accept, this, new_session,
       boost::asio::placeholders::error));
  }

  void handle_accept(session* new_session,
    const boost::system::error_code& error)
  {
    if (!error)
    {
      new_session->start();
      new_session = new session(io_service_);
      acceptor_.async_accept(new_session->socket(),
      boost::bind(&server::handle_accept, this, new_session,
        boost::asio::placeholders::error));
    }
    else
    {
      delete new_session;
    }
  }
};

int main()
{
  try
  {
    boost::asio::io_service io_service;
    static Core core;
    server s(io_service);
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}