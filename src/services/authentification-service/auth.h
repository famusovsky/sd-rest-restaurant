#include "../service.h"
#include <sqlite3.h>

class AuthentificationService : public Service {
public:
    AuthentificationService();

private:
    crow::response post(crow::json::rvalue body) override;
    crow::response get() override;
};