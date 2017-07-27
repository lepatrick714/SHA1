#include <boost/uuid/sha1.hpp>
#include <boost/detail/endian.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
using namespace std;


bool
getFileSH1Hash(const std::string& dirPath, char* buffer)
{
    cout << "Running SH1-Hash on " << dirPath << endl;
    int fd = open(dirPath.c_str(), O_RDONLY);

    // Check if fd is real
    if (fd == -1)
    {
        cout << "ERROR in opening the file!!!" << endl;
        return false;
    }

    vector<char> v(128*1024);
    boost::uuids::detail::sha1 sha1;

    // Reading through the file
    for(;;)
    {
        ssize_t n = read(fd, v.data(), v.size());
        if (n == -1)
        {
            cout << "ERROR READING FILE" << endl;
            return false;
        }
        if (!n)
        {
            break;
        }
        sha1.process_bytes(v.data(), n);
    }
    unsigned hash[5] = {0};
    sha1.get_digest(hash);

    for(int i = 0; i < 5; ++i)
    {
            const char* tmp = reinterpret_cast<char*>(hash);
            buffer[i*4] = tmp[i*4+3];
            buffer[i*4+1] = tmp[i*4+2];
            buffer[i*4+2] = tmp[i*4+1];
            buffer[i*4+3] = tmp[i*4];
    }

    std::cout << "SHA1: " << std::hex;
    for(int i = 0; i < 20; ++i)
    {
        std::cout << ((hash[i] & 0x000000F0) >> 4)
                  <<  (hash[i] & 0x0000000F);
    }
    std::cout << std::endl; // Das wars
}

void display(char* hash)
{
    std::cout << "SHA1: " << std::hex;
    for(int i = 0; i < 20; ++i)
    {
        std::cout << ((hash[i] & 0x000000F0) >> 4)
                  <<  (hash[i] & 0x0000000F);
    }
    std::cout << std::endl; // Das wars
}


int main(int argc, char **argv)
{
  if (argc < 2) { cerr << "Call: " << *argv << " FILE\n"; return 1; }
  const char *filename = argv[1];
  int fd = open(filename, O_RDONLY);
  if (fd == -1) { cerr << "open: " << strerror(errno) << ")\n"; return 1; }
  vector<char> v(128*1024);
  boost::uuids::detail::sha1 sha1;
  for (;;) {
    ssize_t n = read(fd, v.data(), v.size());
    if (n == -1) {
      if (errno == EINTR) continue;
      cerr << "read error: " << strerror(errno) << '\n';
      return 1;
    }
    if (!n) break;
    sha1.process_bytes(v.data(), n);
  }
  static_assert(sizeof(unsigned) == 4, "we are assuming 4 bytes here");
  unsigned hash[5] = {0};
  sha1.get_digest(hash);
#ifdef  BOOST_LITTLE_ENDIAN
  for (unsigned i = 0; i < 5; ++i)
    hash[i] = __builtin_bswap32(hash[i]); // GCC builtin
#endif
  boost::algorithm::hex(boost::make_iterator_range(
        reinterpret_cast<const char*>(hash),
        reinterpret_cast<const char*>(hash+5)),
        std::ostream_iterator<char>(cout)); cout << '\n';
  int r = close(fd);
  if (r == -1) { cerr << "close error: " << strerror(errno) << '\n';
                 return 1; }
  return 0;
}
