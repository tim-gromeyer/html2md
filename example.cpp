#include <iostream>
#include <html2md.hpp>

int main() {
  // Fetch HTML document using curl
  FILE *fp;
  char path[1035];

  fp = popen("curl https://github.com/kstenschke/html2md", "r");

  if (fp ==nullptr) std::cerr << "Failed to run curl command";

  std::string html;

  while (fgets(path, sizeof(path), fp) !=nullptr) html += path;

  pclose(fp);

  // Output conversion result
  std::cout << html2md::Convert(html);

  return 0;
}
