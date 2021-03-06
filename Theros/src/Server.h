#pragma clang diagnostic push
#pragma ide diagnostic ignored "InfiniteRecursion"
#ifndef __SERVER_H__
#define __SERVER_H__

#undef ASIO_HEADER_ONLY
#include "asio.hpp"
#include "asio/impl/src.hpp"
#include "asio/ssl.hpp"
#include "asio/ssl/impl/src.hpp"

#include <fstream>
#include <string>
#include <utility>

#include "Defines.h"
#include "Connection.h"
#include "Router.h"

namespace Theros
{

using ServerAddr = std::pair<std::string, int>;

/**
 * @brief   A generic Http server
 */
template <typename Derived>
class GenericServer
{
public:
  constexpr static int max_header_bytes = 1 << 20; // 1MB
public:
  /* non-copy-constructible */
  GenericServer(const GenericServer &) = delete;
  GenericServer &operator=(const GenericServer &) = delete;

  explicit GenericServer(const ServerAddr server_addr)
      : server_address_(server_addr), io_service_(), acceptor_(io_service_){};

  /**
   * @brief   Starts the server
   *  Initiate io_service event loop,
   *  acceptor instantiates and queues connection
   */
  void run()
  {
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port());

    // configure acceptor
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.set_option(asio::ip::tcp::no_delay(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    /* accpeting connection on an event loop */
    static_cast<Derived *>(this)->accept_connection();
    io_service_.run();
  }

  /**
   * @brief   Getting server address fields
   */
  std::string host() const { return std::get<0>(server_address_); }
  uint16_t port() const { return std::get<1>(server_address_); }

  /**
   * @brief   Scheme + authourity of server
   */
  auto base_url() -> std::string
  {
    return static_cast<Derived *>(this)->scheme() + "://" + host() + ":" +
           std::to_string(port());
  }

public:
  Router router_;
  ServerAddr server_address_; // (host, port) pair
  asio::io_service io_service_;
  asio::ip::tcp::acceptor acceptor_; // tcp acceptor
};

/**
 * @brief   An HTTP server
 */
class HttpServer : public GenericServer<HttpServer>
{
public:
  explicit HttpServer(const ServerAddr server_addr)
      : GenericServer(server_addr){};
  /**
   * @brief   Accept connection and creates new session
   */
  void accept_connection()
  {

    auto new_conn =
        std::make_shared<Connection<TcpSocket>>(io_service_, router_);

    acceptor_.async_accept(
        new_conn->socket_,
        [this, new_conn](std::error_code ec) {

          if (!ec)
          {
            new_conn->start();
          }
          accept_connection();
        });
  }
  /**
   * @brief   Scheme of HTTPS
   */
  std::string scheme() { return "http"; }
};

/**
 * @brief   An HTTPS server
 */
class HttpsServer : public GenericServer<HttpsServer>
{
public:
  explicit HttpsServer(const ServerAddr server_addr)
      : GenericServer(server_addr), context_(asio::ssl::context::sslv23)
  {
    configure_ssl_context();
  };

  /**
   * @brief   Accept connection and creates new session
   */
  void accept_connection()
  {

    auto new_conn =
        std::make_shared<Connection<SslSocket>>(io_service_, context_, router_);

    acceptor_.async_accept(
        new_conn->socket_.lowest_layer(),
        [this, new_conn](std::error_code ec) {
          if (!ec)
          {
            new_conn->start();
          }
          accept_connection();
        });
  }

  /**
   * @brief   Scheme of HTTPS
   */
  std::string scheme() { return "https"; }

private:
  asio::ssl::context context_;

private:
  /**
   * @brief   Sets options, key, cert for Openssl
   */
  void inline configure_ssl_context()
  {
    context_.set_options(asio::ssl::context::default_workarounds |
                         asio::ssl::context::no_sslv2 |
                         asio::ssl::context::single_dh_use);

    context_.set_password_callback([](
        std::size_t max_length, asio::ssl::context::password_purpose purpose) {
      std::string passphrase;
      std::fstream passinf("Summer/ssl/passphrase", std::ios::in);
      if (passinf)
      {
        std::getline(passinf, passphrase);
      }
      return passphrase;
    });
    context_.use_private_key_file("Summer/ssl/key.pem", asio::ssl::context::pem);
    context_.use_certificate_chain_file("Summer/ssl/cert.pem");
    context_.use_tmp_dh_file("Summer/ssl/dh512.pem");
  };
};

} // namespace Theros
#endif // __SERVER_H__
#pragma clang diagnostic pop