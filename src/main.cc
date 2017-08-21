/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of drsd nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "unistd.h"

#include "command_parser.h"

using namespace rdp_tool;

const char *instructing_information =
    "\n"
    "Usage: %s -b <host1:port1,host2:port2,..>\n "
    "\n";

int main(int argc, char **argv) {
  std::string brokers;
  int opt;
  while ((opt = getopt(argc, argv, "b:")) != -1) {
    switch (opt) {
      case 'b':
        brokers = optarg;
        break;
      default:
        break;
    }
  }

  if (optind != argc || brokers.empty()) {
    fprintf(stderr, instructing_information, argv[0]);
    exit(-1);
  }

  printf("\n");
  printf("broker : '%s'\n", brokers.c_str());
  printf("\n");

  ZookeeperHandle zoo_handle;
  CommandParser command_parser;

  if (!zoo_handle.ZookeeperInit(brokers.c_str(), 30000)) {
    fprintf(stderr, "%s %s\n", "Error in connect to ", brokers.c_str());
    return -1;
  };

  if (!command_parser.CommandParserInit(&zoo_handle)) {
    fprintf(stderr, "%s\n", "Error init commandparser");
    return -1;
  }
  command_parser.CommandParserRun();
}
