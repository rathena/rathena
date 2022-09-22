
#ifndef discord_request_hpp
#define discord_request_hpp

#include <curl/curl.h>
#include <string>

namespace rocord {
class http_request
{
public:
  /* request object contains all data to perfrom an api call to discord.
   * header: a pointer to an curl object required to perfrom the request.
   * url: url for the request.
   * content: data for the request (has to be in the correct format
   * 			for http requests).
   */
  http_request(curl_slist* header, std::string& url, std::string& content,
               std::string& type)
    : header(header)
    , url(url)
    , content(content)
    , type(type){};
  http_request(){};
  virtual ~http_request(){};

  curl_slist* header;
  std::string url;
  std::string content;
  std::string type;
};
}

#endif // discord_request_hpp
