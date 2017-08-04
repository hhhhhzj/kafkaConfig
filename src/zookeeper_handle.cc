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

#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <fstream>
#include <iostream>

#include <jansson.h>

#include "zookeeper_handle.h"

namespace rdp_tool {

ZookeeperHandle::ZookeeperHandle() {
  zk_handle_ = NULL;
  zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
}

ZookeeperHandle::~ZookeeperHandle() {
  if (zk_handle_) {
    int ret = zookeeper_close(zk_handle_);

    if (ZOK != ret) {
      ZookeeperError(ret, "zookeeper_close");
    }
  }
}

bool ZookeeperHandle::ZookeeperInit(const char *host, int timeout) {
  zk_handle_ = zookeeper_init(host, NULL, timeout, 0, NULL, 0);
  if (NULL == zk_handle_) {
    fprintf(stderr, "Error when connecting to zookeeper servers...\n");
    return false;
  }
  return true;
}

void ZookeeperHandle::ListFiles(const char *path) {
  int ret = 0;
  static String_vector dir_strings;

  ret = zoo_get_children2(zk_handle_, path, 0, &dir_strings, &node_stat_);
  if (ZOK != ret) {
    fprintf(stderr, "Error when list files...\n");
    return;
  }

  char tctimes[40];
  char tmtimes[40];
  time_t tctime;
  time_t tmtime;
  memset(tctimes, 0, 40);
  memset(tmtimes, 0, 40);

  tctime = node_stat_.ctime / 1000;
  tmtime = node_stat_.mtime / 1000;

  ctime_r(&tmtime, tmtimes);
  ctime_r(&tctime, tctimes);

  fprintf(stdout,
          "[\n  ctime = %s  czxid=%llx\n"
          "  mtime = %s  mzxid=%llx\n"
          "  version = %x aversion = %x\n]\n",
          tctimes, node_stat_.czxid, tmtimes, node_stat_.mzxid,
          (unsigned int)node_stat_.version, (unsigned int)node_stat_.aversion);

  int dir_count = dir_strings.count;
  char **iter = dir_strings.data;
  for (int i = 0; i < dir_count; i++) {
    fprintf(stdout, "%s\n", *(iter + i));
  }
}

void ZookeeperHandle::GetNode(const char *path) {
  int ret = 0;
  char *data_buffer = new char[1000000];
  int buffer_len = 1000000;
  json_t *json_node_value = NULL;
  json_error_t error;

  ret = zoo_exists(zk_handle_, path, 0, &node_stat_);
  if (ZOK != ret) {
    ZookeeperError(ret, "zoo_exists");
    return;
  }

  memset(data_buffer, 0, 1000000);
  ret = zoo_get(zk_handle_, path, 0, data_buffer, &buffer_len, &node_stat_);
  if (buffer_len > 1000000) {
    fprintf(stdout, "%s\n", "data_buffer is too small");
  }
  if (ZOK != ret) {
    ZookeeperError(ret, "zoo_get");
    return;
  }

  bool is_json = true;
  json_node_value = json_loads(data_buffer, 0, &error);

  system("echo \"\" > node_data_temp.json");

  if (json_node_value) {
    json_dump_file(json_node_value, "./node_data_temp.json",
                   JSON_INDENT(4) | JSON_COMPACT | JSON_PRESERVE_ORDER);
  } else {
    is_json = false;
    FILE *stream = fopen("./node_data_temp.json", "w");
    if (NULL == stream) {
      fprintf(stderr, "%s %s\n", "Error open file node_data_temp.json, ",
              strerror(errno));
      return;
    }
    fprintf(stream, "%s", data_buffer);
    fclose(stream);
  }

  bool json_check = true;

  while (json_check) {
    system("vim node_data_temp.json");

    memset(data_buffer, 0, 1000000);
    FILE *stream = fopen("./node_data_temp.json", "r");
    if (NULL == stream) {
      fprintf(stderr, "%s %s\n", "Error open file node_data_temp.json, ",
              strerror(errno));
      return;
    }
    fread(data_buffer, 1000000, sizeof(char), stream);
    fclose(stream);

    // json格式校验
    if (is_json) {
      json_node_value = json_loads(data_buffer, 0, &error);
      if (!json_node_value) {
        fprintf(stderr, "Json format error on line %d: %s\n", error.line,
                error.text);
        char ch;
        fprintf(stdout, "%s: ", "Continue? Y or N");
        ch = getchar();
        putchar(ch);
        if ('Y' == ch || 'y' == ch) {
          json_check = false;
        }
        //去除回车符
        getchar();
        continue;
      }

      data_buffer = json_dumps(json_node_value, 0);
    }
    json_check = false;
  }

  printf("%s\n", data_buffer);
  ret = zoo_set(zk_handle_, path, data_buffer, strlen(data_buffer), -1);
  if (ZOK != ret) {
    ZookeeperError(ret, "zoo_set");
  }
  delete[] data_buffer;
  json_decref(json_node_value);
}

bool ZookeeperHandle::DelNode(const char *path) {
  int ret = 0;
  ret = zoo_delete(zk_handle_, path, -1);
  if (ZOK != ret) {
    ZookeeperError(ret, "zoo_delete");
    return false;
  }

  return true;
}

bool ZookeeperHandle::SetNode(std::string node_path,
                              const std::string &node_value, int mod) {
  CreateNode(node_path, mod);
  if (!IsExist(node_path.c_str())) {
    return false;
  }

  int ret = 0;
  ret = zoo_set(zk_handle_, node_path.c_str(), node_value.c_str(),
                node_value.size(), -1);
  if (ZOK != ret) {
    ZookeeperError(ret, "zoo_create");
    return false;
  }

  return true;
}

void ZookeeperHandle::CreateNode(std::string path, int mode) {
  if (IsExist(path.c_str()) || path.empty()) return;

  if (1 == mode) {
    std::string sub_path;
    int sprit_pos_last = path.find_last_of('/');

    if (0 != sprit_pos_last) {
      sub_path = path.substr(0, sprit_pos_last);
    }

    CreateNode(sub_path, 1);
  }

  int ret = 0;
  ret = zoo_create(zk_handle_, path.c_str(), NULL, -1, &ZOO_OPEN_ACL_UNSAFE, 0,
                   NULL, 0);
  if (ZOK != ret) {
    ZookeeperError(ret, "zoo_create");
  }
}

bool ZookeeperHandle::LoadFromFile(const char *text_path) {
  if (NULL == text_path) {
    fprintf(stderr, "%s\n", "Error  the file name is NULL");
    return false;
  }

  std::ifstream text_fp;
  text_fp.open(text_path, std::fstream::in);
  if (!text_fp.is_open()) {
    fprintf(stderr, "%s %s %s\n", "Error open file: ", text_path, strerror(errno));
    return false;
  }

  std::string node_path_value;
  bool err_flag = true;

  while (getline(text_fp, node_path_value)) {
    if (node_path_value.empty()) continue;
    int equal_pos_first = node_path_value.find_first_of('=');
    std::string node_path, node_value;
    if (0 != equal_pos_first) {
      node_path = node_path_value.substr(0, equal_pos_first);
    }

    if (std::string::npos != equal_pos_first) {
      node_value = node_path_value.substr(equal_pos_first + 1);
    }

    if (!SetNode(node_path, node_value, 1)) {
      fprintf(stderr, "%s %s\n", "Error set the node: ", node_path.c_str());
      err_flag = false;
    }
  }

  return err_flag;
}

bool ZookeeperHandle::IsExist(const char *path) {
  int ret = zoo_exists(zk_handle_, path, 0, &node_stat_);
  if (ZOK != ret) {
    return false;
  }
  return true;
}

void ZookeeperHandle::ZookeeperError(int ret, const char *function) {
  switch (ret) {
    case ZBADARGUMENTS:
      fprintf(stderr, "%s\n", "Invalid arguments");
      break;
    case ZSYSTEMERROR:
    case ZRUNTIMEINCONSISTENCY:
      fprintf(stderr, "%s\n", "A runtime inconsistency was found");
      break;
    case ZOPERATIONTIMEOUT:
      fprintf(stderr, "%s\n", "Operation timeout");
      break;
    case ZNONODE:
      fprintf(stderr, "%s\n", "Node does not exist");
      break;
    case ZNODEEXISTS:
      fprintf(stderr, "%s\n", "The node already exists");
      break;
    case ZCLOSING:
      fprintf(stderr, "%s\n", "The zookeeper is closing");
      break;
    default:
      fprintf(stderr, "%s  Error: %d\n", function, ret);
      break;
  }
}

}  // namespace rdp_tool
