# Домашнее задание №4

# [Система обработки заказов ресторана](https://clck.ru/34XSGw)

## Степанов Алексей, БПИ212

### Required: 
- [C++](https://isocpp.org/)
- [CMake](https://cmake.org/)
- [Crow](https://github.com/CrowCpp/Crow)
- [CPR](https://github.com/libcpr/cpr)
- [SQLite](https://www.sqlite.org/)

### Compile and Run
```bash
# Compile
cmake . -B build && cmake --build build --target all
 
# Run
./build/src/main
```
> From the project's root directory

### IT IS WORKING!!!



### Sample requests

```bash
curl -X POST -H "Content-Type: application/json" -d '{"username": "dipper", "email": "pines@ya.ru", "password": "qwerty", "role": "manager"}' http://localhost:8080/auth/sign-up

curl -X POST -H "Content-Type: application/json" -d '{"username": "dipper", "password": "qwerty", "session_token": "12345"}' http://localhost:8080/auth/sign-in

curl -X POST -H "Content-Type: application/json" -d '{"session_token": "12345", "dish_name": "pizza", "quantity": "50"}' http://localhost:8080/orders/manage-dishes

curl -X POST -H "Content-Type: application/json" -d '{"username": "bebra", "email": "amog@us.me", "password": "sussy_baka", "role": "customer"}' http://localhost:8080/auth/sign-up

curl -X POST -H "Content-Type: application/json" -d '{"username": "bebra", "password": "sussy_baka", "session_token": "54321"}' http://localhost:8080/auth/sign-in

curl -X POST -H "Content-Type: application/json" -d '{"session_token": "54321", "dish_name": "pizza", "quantity": "5"}' http://localhost:8080/orders/create

curl -X GET -H "Content-Type: application/json" -d '{"session_token": "12345", "order_id": "1"}' http://localhost:8080/orders

curl -X GET -H "Content-Type: application/json" -d '{"session_token": "12345"}' http://localhost:8080/auth

curl -X POST -H "Content-Type: application/json" -d '{"session_token": "12345"}' http://localhost:8080/auth/sign-out

curl -X GET -H "Content-Type: application/json" -d '{"session_token": "12345"}' http://localhost:8080/auth
```