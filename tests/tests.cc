#ifndef CATCH_CONFIG_MAIN
#  define CATCH_CONFIG_MAIN
#endif

#include "atm.hpp"
#include "catch.hpp"
// testing

/////////////////////////////////////////////////////////////////////////////////////////////
//                             Helper Definitions //
/////////////////////////////////////////////////////////////////////////////////////////////

bool CompareFiles(const std::string& p1, const std::string& p2) {
  std::ifstream f1(p1);
  std::ifstream f2(p2);

  if (f1.fail() || f2.fail()) {
    return false;  // file problem
  }

  std::string f1_read;
  std::string f2_read;
  while (f1.good() || f2.good()) {
    f1 >> f1_read;
    f2 >> f2_read;
    if (f1_read != f2_read || (f1.good() && !f2.good()) ||
        (!f1.good() && f2.good()))
      return false;
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Test Cases
/////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Example: Create a new account", "[ex-1]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);

  auto accounts = atm.GetAccounts();
  REQUIRE(accounts.contains({12345678, 1234}));
  REQUIRE(accounts.size() == 1);

  Account sam_account = accounts[{12345678, 1234}];
  REQUIRE(sam_account.owner_name == "Sam Sepiol");
  REQUIRE(sam_account.balance == 300.30);

  auto transactions = atm.GetTransactions();
  REQUIRE(transactions.contains({12345678, 1234}));
  REQUIRE(transactions.size() == 1);
  std::vector<std::string> empty;
  REQUIRE(transactions[{12345678, 1234}] == empty);

  REQUIRE_THROWS_AS(atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30),
                    std::invalid_argument);
}

TEST_CASE("Example: Simple widthdraw", "[ex-2]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);
  atm.RegisterAccount(22222222, 2222, "Dummy", 0);

  REQUIRE_THROWS_AS(atm.WithdrawCash(22222222, 2222, -1),
                    std::invalid_argument);
  REQUIRE_THROWS_AS(atm.WithdrawCash(22222222, 2222, 1), std::runtime_error);
  REQUIRE_THROWS_AS(atm.WithdrawCash(11111111, 1111, 1), std::invalid_argument);

  atm.WithdrawCash(12345678, 1234, 20);

  REQUIRE(atm.CheckBalance(12345678, 1234) == 280.30);
  REQUIRE_THROWS_AS(atm.CheckBalance(12345671, 1234), std::invalid_argument);
}

TEST_CASE("Example: Simple Deposit", "[ex-3]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);
  atm.RegisterAccount(22222222, 2222, "Dummy", 0);

  REQUIRE_THROWS_AS(atm.DepositCash(22222222, 2222, -1), std::invalid_argument);
  REQUIRE_THROWS_AS(atm.DepositCash(11111111, 1111, 1), std::invalid_argument);

  atm.DepositCash(12345678, 1234, 20);
  auto accounts = atm.GetAccounts();
  Account sam_account = accounts[{12345678, 1234}];

  REQUIRE(sam_account.balance == 320.30);

  auto transactions = atm.GetTransactions();
  REQUIRE(transactions[{12345678, 1234}].size() == 1);
}

TEST_CASE("Example: Print Prompt Ledger", "[ex-4]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);
  auto& transactions = atm.GetTransactions();
  transactions[{12345678, 1234}].push_back(
      "Withdrawal - Amount: $200.40, Updated Balance: $99.90");
  transactions[{12345678, 1234}].push_back(
      "Deposit - Amount: $40000.00, Updated Balance: $40099.90");
  transactions[{12345678, 1234}].push_back(
      "Deposit - Amount: $32000.00, Updated Balance: $72099.90");

  atm.PrintLedger("./prompt.txt", 12345678, 1234);
  REQUIRE_THROWS_AS(atm.PrintLedger("./prompt.txt", 11111111, 1239),
                    std::invalid_argument);

  std::string expected_ledger =
      "Name: Sam Sepiol\n"
      "Card Number: 12345678\n"
      "PIN: 1234\n"
      "----------------------------\n"
      "Withdrawal - Amount: $200.40, Updated Balance: $99.90\n"
      "Deposit - Amount: $40000.00, Updated Balance: $40099.90\n"
      "Deposit - Amount: $32000.00, Updated Balance: $72099.90";

  REQUIRE(CompareFileToString("./prompt.txt", expected_ledger));
}