#include <iostream>

#include "document.h"
#include "paginator.h"
#include "read_input_functions.h"
#include "request_queue.h"
#include "search_server.h"

int main() {
    using namespace std::literals::string_literals;

    SearchServer search_server = SearchServer("and in at"s);
    RequestQueue request_queue(search_server);

    AddDocument(search_server, 1, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
    AddDocument(search_server, 2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
    AddDocument(search_server, 3, "big cat fancy collar "s, DocumentStatus::ACTUAL, {1, 2, 8});
    AddDocument(search_server, 4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, {1, 3, 2});
    AddDocument(search_server, 5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, {1, 1, 1});

    // 1439 запросов с нулевым результатом
    for (int i = 0; i < 1439; ++i) {
        request_queue.AddFindRequest("empty request"s);
    }
    // все еще 1439 запросов с нулевым результатом
    request_queue.AddFindRequest("curly dog"s);
    // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
    request_queue.AddFindRequest("big collar"s);
    // первый запрос удален, 1437 запросов с нулевым результатом
    request_queue.AddFindRequest("sparrow"s);
    std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;
    
} 