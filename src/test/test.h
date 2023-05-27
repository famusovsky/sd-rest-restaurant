#include <vector>
#include <string>
#include <cpr/cpr.h>
#include <crow.h>

class Test {
public:
    Test();

    void init(crow::SimpleApp& app);

private:
    std::vector<std::string> strings_;

    crow::response post(crow::json::rvalue body);

    crow::response get();
};