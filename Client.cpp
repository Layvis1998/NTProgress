#include <iostream>
#include <boost/asio.hpp>

#include "Common.hpp"
#include "json.hpp"

using boost::asio::ip::tcp;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

enum state {main_menu, start_menu};
state now = start_menu;
string my_id;
string my_psw;

// Отправка сообщения на сервер по шаблону.
void SendMessage(
    tcp::socket& aSocket,
    const string& aId,
    const string& aRequestType,
    const string& aMessage)
{
  nlohmann::json req;
  req["UserId"] = aId;
  req["ReqType"] = aRequestType;
  req["Message"] = aMessage;

  string request = req.dump();
  boost::asio::write(aSocket, boost::asio::buffer(request, request.size()));
}

// Возвращает строку с ответом сервера на последний запрос.
string ReadMessage(tcp::socket& aSocket)
{
  boost::asio::streambuf b;
  boost::asio::read_until(aSocket, b, "\0");

  std::istream is(&b);
  string line(std::istreambuf_iterator<char>(is), {});
  return line;
}


// "Создаём" пользователя, получаем его ID.
void GetName(tcp::socket& aSocket)
{
  SendMessage(aSocket, my_id, Requests::Hello, "");
  cout << ReadMessage(aSocket);
}

// "Создаём" пользователя, получаем его ID.
string ProcessRegistration(tcp::socket& aSocket)
{
  string name;
  cout << "Hello! Enter your name: ";
  cin >> name;

  // Для регистрации Id не нужен, заполним его нулём
  SendMessage(aSocket, "0", Requests::Registration, name);
  return ReadMessage(aSocket);
}

// "Создаём" пароль пользователя
void CreatePassword(tcp::socket& aSocket)
{
  string password;
  cout << "Enter your password: ";
  cin >> password;


  SendMessage(aSocket, my_id, Requests::CreatePassword, password);
  cout << "Your new password is: ";
  cout << ReadMessage(aSocket) << endl;
}

void GetBalance(tcp::socket& aSocket)
{ 
  SendMessage(aSocket, my_id, Requests::Balance, "");
  cout << ReadMessage(aSocket);
}

string CheckID(tcp::socket& aSocket)
{
  cout << "Enter your ID: " << endl;
  cin >> my_id;
  try
  {
    stoi(my_id);
  }
  catch(std::exception& e)
  {
    cout << "Please, enter a valid id integer number!" << endl;
    return "incorrect";
  }
    
  SendMessage(aSocket, my_id, Requests::CheckID, "");
  return ReadMessage(aSocket);
}

// Заходим на аккаунт по ID и паролю
string SignIn(tcp::socket& aSocket)
{
  string password;
  cout << "Enter your password: " << endl;
  cin >> password;

  SendMessage(aSocket, my_id, Requests::CheckPassword, password);
  return ReadMessage(aSocket);
}

string CreateBuyApp(tcp::socket& aSocket)
{
  string amount;  
  string price;
  try
  {
    cout << "Enter the amount of dollars you want to buy: " << endl;
    cin >> amount;

    cout << "Enter the price you are ready to pay per dollar: " << endl;
    cin >> price;
    if ((stoi(price) <= 0) || (stoi(amount) <= 0))
    {
      cout << "Enter positive numbers, please!" << endl;
      return "";
    }
  }
  catch (std::exception& e)
  {
    cout << "Enter valid numbers in your app, please!" << endl;
    return "";
  }

  SendMessage(aSocket, my_id, Requests::BuyApp, amount + " " + price);
  return ReadMessage(aSocket);

}

string CreateSellApp(tcp::socket& aSocket)
{
  string amount;  
  string price;
  try
  {
    cout << "Enter the amount of dollars you want to sell: " << endl;
    cin >> amount;

    cout << "Enter the price at which you want to sell one dollar: " << endl;
    cin >> price;
    if ((stoi(price) <= 0) || (stoi(amount) <= 0))
    {
      cout << "Enter positive numbers, please!" << endl;
      return "";
    }
  }
  catch (std::exception& e)
  {
    cout << "Enter valid numbers in your app, please!" << endl;
    return "";
  }

  SendMessage(aSocket, my_id, Requests::SellApp, amount + " " + price);
  return ReadMessage(aSocket);
  
}

void CancellApp(tcp::socket& aSocket)
{
  string amount;  
  string price;
  string type;
  try
  {
    cout << "Enter the amount of dollars in your app: " << endl;
    cin >> amount;

    cout << "Enter the price in your app: " << endl;
    cin >> price;

    if ((stoi(price) <= 0) || (stoi(amount) <= 0))
    {
      cout << "Enter positive numbers, please!" << endl;
      return;
    }

    cout << "Enter \"+\" symbol if it is a buy app or \"-\""
         << " if it is a sell app\n";
    cin >> type;

    if ((type != "+") && (type != "-"))
      return;

  }
  catch (std::exception& e)
  {
    cout << "Enter valid numbers in your app, please!" << endl;
    return;
  }

  SendMessage(aSocket, my_id, Requests::CancelApp,
              amount + " " + price + " " + type);
  cout << ReadMessage(aSocket);
  
}

void DisplayAllApps(tcp::socket& aSocket)
{
  SendMessage(aSocket, my_id, Requests::ShowDOM, "");
  cout << ReadMessage(aSocket);
}

void DisplayMyApps(tcp::socket& aSocket)
{
  SendMessage(aSocket, my_id, Requests::ActiveApps, "");
  cout << ReadMessage(aSocket);
}

void DisplayMyDeals(tcp::socket& aSocket)
{
  SendMessage(aSocket, my_id, Requests::MyDeals, "");
  cout << ReadMessage(aSocket);
}

void StartMenu(tcp::socket& s)
{
  while (true)
  {
    // Тут реализовано "бесконечное" меню.
    cout << "Start menu:\n"
            "1) Sign up\n"
            "2) Log in\n"
            "3) Exit\n"
         << endl;

    string menu_option_str;
    int menu_option_num = 0;
    cin >> menu_option_str;
    try
    {
      menu_option_num = stoi(menu_option_str);
    }

    catch (std::exception& e)
    {
      cout << "Enter valid number\n";
    }

    switch (menu_option_num)
    {
      case 1:
      {
        // Мы предполагаем, что для идентификации пользователя будет использоваться ID.
        // Тут мы "регистрируем" пользователя - отправляем на сервер имя, а сервер возвращает нам ID.
        // Этот ID далее используется при отправке запросов.
       
        my_id = ProcessRegistration(s);
        cout << "Your new ID is: " << my_id << endl;
        CreatePassword(s);
        break;
      }
      case 2:
      {
        if ((CheckID(s) == "correct") && (SignIn(s) == "correct"))
        {
          now = main_menu;
          return;
        }
        else
          cout << "Incorrect ID or password " << endl;

        break;
      }
      case 3:
      {
        exit(0);
        break;
      }
      default:
      {
        cout << "Unknown menu option\n" << endl;
        break;
      }
    }
  }
}

void MainMenu(tcp::socket& s)
{
  while (true)
  {
    // Тут реализовано "бесконечное" меню.
    cout << "Main menu:\n"
            "0) Get user name\n"
            "1) Get balance\n"
            "2) Create buy app\n"
            "3) Create sell app\n"
            "4) Cancel app\n"
            "5) Show my apps\n"
            "6) Show DOM\n"
            "7) Show my bargains\n"
            "8) Log out\n"
            "9) Exit\n"
         << endl;


    string menu_option_str;
    int menu_option_num = 0;
    cin >> menu_option_str;
    try
    {
      menu_option_num = stoi(menu_option_str);
    }

    catch (std::exception& e)
    {
      cout << "Enter valid number\n";
    }

    switch (menu_option_num)
    {
      case 0:
      {
        GetName(s);
        break;
      }
      case 1:
      {
        GetBalance(s);
        break;
      }
      case 2:
      {
        CreateBuyApp(s);
        break;
      }
      case 3:
      {
        CreateSellApp(s);
        break;
      }
      case 4:
      {
        CancellApp(s);
        break;
      }
      case 5:
      {
        DisplayMyApps(s);
        break;
      }
      case 6:
      {
        DisplayAllApps(s);
        break;
      }
      case 7:
      {
        DisplayMyDeals(s);
        break;
      }
      case 8:
      {
        now = start_menu;
        return;
      }
      case 9:
      {
        exit(0);
      }
      default:
      {
        cout << "Unknown menu option\n" << endl;
        break;
      }
    }
  }    
}

int main()
{
  try
  {
    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(tcp::v4(), "127.0.0.1", std::to_string(port));
    tcp::resolver::iterator iterator = resolver.resolve(query);

    tcp::socket s(io_service);
    s.connect(*iterator);
    string my_id;
    
    while (1)
    {    
      switch(now)
      {
        case start_menu:
          StartMenu(s);
        case main_menu:
          MainMenu(s);
      }
    }
  }
  catch (std::exception& e)
  {
    cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}