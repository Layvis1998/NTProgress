#ifndef CLIENSERVERECN_COMMON_HPP
#define CLIENSERVERECN_COMMON_HPP



static short port = 5555;

namespace Requests
{
  static std::string Registration = "Reg";
  static std::string Hello = "Hel";
  static std::string CreatePassword = "CRPSW";
  static std::string CheckPassword = "CHPSW";
  static std::string CheckID = "CID";
  static std::string Balance = "BLC";
  static std::string BuyApp = "BAPP";
  static std::string CancelApp = "CAPP";
  static std::string SellApp = "SAPP";
  static std::string ActiveApps = "AAPP";
  static std::string ShowDOM = "SDOM";
  static std::string MyDeals = "MD";
}

#endif //CLIENSERVERECN_COMMON_HPP
