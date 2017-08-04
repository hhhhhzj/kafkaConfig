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

#include <iostream>

#include "command_parser.h"

namespace rdp_tool {

const char *command_help =
    "ls       --List information about the Nodes (the "
    "current directory by default).\n"
    "cd       --Switch to the specified directory\n"
    "pwd      --Print current directory\n"
    "mkdir    --Create a new node\n"
    "rm       --Delete the specified directory\n"
    "vim      --Edit node data\n"
    ".        --The current directory\n"
    "..       --The upper directory\n"
    "exit     --Exit the program\n"
    "source   --Import nodedatas from File\n"
    "help     --Print information\n";

CommandParser::CommandParser() {
  current_path_ = "/";
  zookeeper_handle_ = NULL;
}

bool CommandParser::CommandParserInit(ZookeeperHandle *zookeeper_handle) {
  zookeeper_handle_ = zookeeper_handle;
  if (NULL == zookeeper_handle) {
    fprintf(stderr, "%s\n", "Error the zookeeper is NULL");
    return false;
  }
  return true;
}

void CommandParser::CommandParserRun() {
  std::string command_line;

  bool flag_exit = false;
  while (!flag_exit) {
    std::cout << "root@zookeeper:" << current_path_ << "$ ";

    getline(std::cin, command_line);
    Trim(command_line);

    if ("exit" == command_line) {
      flag_exit = true;
      continue;
    }

    std::string prefix_command;
    std::string postfix_command;

    int space_pos = command_line.find_first_of(' ');
    if (0 != space_pos) {
      prefix_command = Trim(command_line.substr(0, space_pos));
    }
    if (std::string::npos != space_pos) {
      postfix_command = Trim(command_line.substr(space_pos + 1));
    }

    if ("ls" == prefix_command) {
      std::string full_path = GetFullPath(postfix_command);

      zookeeper_handle_->ListFiles(full_path.c_str());

    } else if ("source" == prefix_command) {
      std::string file_path = postfix_command;

      if (zookeeper_handle_->LoadFromFile(file_path.c_str())) {
        fprintf(stdout, "%s %s\n", "Success source from file: ",
                file_path.c_str());
      } else {
        fprintf(stdout, "%s %s\n", "Error source from file: ",
                file_path.c_str());
      }

    }

    else if ("cd" == prefix_command) {
      std::string full_path = GetFullPath(postfix_command);
      if (zookeeper_handle_->IsExist(full_path.c_str())) {
        current_path_ = full_path;
      }

    } else if ("pwd" == prefix_command) {
      std::cout << current_path_ << std::endl;

    } else if ("vim" == prefix_command) {
      std::string full_path = GetFullPath(postfix_command);
      zookeeper_handle_->GetNode(full_path.c_str());

    } else if ("mkdir" == prefix_command) {
      std::string full_path = GetFullPath(postfix_command);
      zookeeper_handle_->CreateNode(full_path.c_str(), 0);

    } else if ("rm" == prefix_command) {
      std::string full_path = GetFullPath(postfix_command);
      zookeeper_handle_->DelNode(full_path.c_str());

    } else {
      fprintf(stdout, "%s", command_help);
    }
  }
}

std::string CommandParser::GetFullPath(std::string path) {
  std::string full_path = current_path_;

  std::string dir_name;
  while (!path.empty()) {
    int sprit_pos_first = path.find_first_of('/');
    dir_name = path.substr(0, sprit_pos_first);

    if (std::string::npos != sprit_pos_first) {
      path = path.substr(sprit_pos_first + 1);
    } else {
      path = "";
    }

    if (0 == sprit_pos_first) {
      full_path = "/";
      continue;
    }

    if ("." == dir_name) {
      continue;

    } else if (".." == dir_name) {
      int sprit_pos_last = full_path.find_last_of("/");
      full_path = full_path.substr(0, sprit_pos_last);
      if ("" == full_path) {
        full_path = "/";
      }

    } else {
      if ("/" != full_path) full_path = full_path + "/";
      full_path = full_path + dir_name;
    }
  }

  return full_path;
}
}  // namespace rdp_tool
