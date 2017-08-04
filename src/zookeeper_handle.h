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

#ifndef RDP_SRC_ZOOKEEPERHANDLE_H
#define RDP_SRC_ZOOKEEPERHANDLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include <zookeeper/zookeeper.h>
#include <zookeeper/zookeeper_log.h>

namespace rdp_tool {

class ZookeeperHandle {
 public:
  explicit ZookeeperHandle();
  ~ZookeeperHandle();
  bool ZookeeperInit(const char *host, int timeout);
  void ListFiles(const char *path);
  void GetNode(const char *path);
  bool DelNode(const char *path);
  bool SetNode(std::string node_path, const std::string &node_value,
               int mod = 0);

  //创建新节点，如果父节点不存在，mode 0则创建失败, mode 1 先递归创建父节点
  void CreateNode(std::string path, int mode_t = 0);

  bool LoadFromFile(const char *text_path);
  bool IsExist(const char *path);

 private:
  void ZookeeperError(int ret, const char *function);
  zhandle_t *zk_handle_;
  struct Stat node_stat_;
};

}  // namespace rdp_tool
#endif  // RDP_SRC_ZOOKEEPERHANDLE_H
