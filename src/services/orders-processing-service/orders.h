#include "../service.h"

class OrdersProcessingService : public Service {
public:
    OrdersProcessingService();

private:
    crow::response post(crow::json::rvalue body) override;
    crow::response get() override;
};