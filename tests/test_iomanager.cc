#include "../link/links.h"
#include "../link/iomanager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

links::Logger::ptr g_logger = LINK_LOG_ROOT();

int sock = 0;

void test_fiber() {
    LINK_LOG_INFO(g_logger) << "test_fiber";
    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "183.2.172.185", &addr.sin_addr.s_addr);

    if (!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {

    } else if (errno == EINPROGRESS) {
        LINK_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        links::IOManager::GetThis()->addEvent(sock, links::IOManager::READ, [](){
            LINK_LOG_INFO(g_logger) << "read callback";
        });
        links::IOManager::GetThis()->addEvent(sock, links::IOManager::WRITE, [](){
            LINK_LOG_INFO(g_logger) << "write callback";
            links::IOManager::GetThis()->cancelEvent(sock, links::IOManager::READ);
            close(sock);
        });
    }else {
        LINK_LOG_INFO(g_logger) << "else " << errno << " " <<strerror(errno);
    }

}

void test1() {
    links::IOManager iom(2, false);
    iom.schedule(&test_fiber);
}

int main(int argc, char** argv) {
    test1();
    return 0;
}
