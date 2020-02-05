/* http_server.cc
   Mathieu Stefani, 07 février 2016

   Example of an http server
*/

#include <pistache/net.h>
#include <pistache/http.h>
#include <pistache/peer.h>
#include <pistache/http_headers.h>
#include <pistache/cookie.h>
#include <pistache/endpoint.h>
#include <pistache/common.h>

using namespace std;
using namespace Pistache;

struct LoadMonitor {
    LoadMonitor(const std::shared_ptr<Http::Endpoint>& endpoint)
        : endpoint_(endpoint)
        , interval(std::chrono::seconds(1))
    { }

    void setInterval(std::chrono::seconds secs) {
        interval = secs;
    }

    void start() {
        shutdown_ = false;
        thread.reset(new std::thread(std::bind(&LoadMonitor::run, this)));
    }

    void shutdown() {
        shutdown_ = true;
    }

    ~LoadMonitor() {
        shutdown_ = true;
        if (thread) thread->join();
    }

private:
    std::shared_ptr<Http::Endpoint> endpoint_;
    std::unique_ptr<std::thread> thread;
    std::chrono::seconds interval;

    std::atomic<bool> shutdown_;

    void run() {
        Tcp::Listener::Load old;
        while (!shutdown_) {
            if (!endpoint_->isBound()) continue;

            endpoint_->requestLoad(old).then([&](const Tcp::Listener::Load& load) {
                old = load;

                double global = load.global;
                if (global > 100) global = 100;

                if (global > 1)
                    std::cout << "Global load is " << global << "%" << std::endl;
                else
                    std::cout << "Global load is 0%" << std::endl;
            },
            Async::NoExcept);

            std::this_thread::sleep_for(std::chrono::seconds(interval));
        }
    }
};


class MyHandler : public Http::Handler {

    HTTP_PROTOTYPE(MyHandler)

    void onRequest(
            const Http::Request& req,
            Http::ResponseWriter response) override {

        if (req.resource() == "/") {
            if (req.method() == Http::Method::Get) {
                Http::serveFile(response, "index.html").then([](ssize_t bytes) {
                    std::cout << "Sent " << bytes << " bytes" << std::endl;
                }, Async::NoExcept);
            }
        } else {
            if (req.method() == Http::Method::Get) {
                std::string path(std::move(req.resource()));
                path.erase(path.begin());
                std::cout << path << std::endl;
                Http::serveFile(response, path).then([](ssize_t bytes) {
                    std::cout << "Sent " << bytes << " bytes" << std::endl;
                }, Async::NoExcept);
            }
        }
    }

    void onTimeout(
            const Http::Request& req,
            Http::ResponseWriter response) override {
        UNUSED(req);
        response
            .send(Http::Code::Request_Timeout, "Timeout")
            .then([=](ssize_t) { }, PrintException());
    }

};

int main(int argc, char *argv[]) {
    Port port(9080);

    int thr = 2;

    if (argc >= 2) {
        port = std::stol(argv[1]);

        if (argc == 3)
            thr = std::stol(argv[2]);
    }

    Address addr(Ipv4::any(), port);

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;

    auto server = std::make_shared<Http::Endpoint>(addr);
    auto flags = Tcp::Options::ReuseAddr;

    auto opts = Http::Endpoint::options()
        .threads(thr)
        .flags(flags);
    server->init(opts);
    server->setHandler(Http::make_handler<MyHandler>());
//    server->useSSL("./certs/server.crt", "./certs/server.key");
    server->serve();
}
