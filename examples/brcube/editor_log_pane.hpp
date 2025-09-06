#ifndef EDITOR_LOG_PANE_HPP_
#define EDITOR_LOG_PANE_HPP_

#include <vector>
#include "editor_common.hpp"

namespace Editor
{

class LogPane
{
public:
    explicit LogPane(const char *id);

    void Draw();

    void SetSource(const std::vector<LogEntry> *source);

private:
    br_res_ptr<char>             m_id;
    const std::vector<LogEntry> *m_source;
};

} // namespace Editor

#endif /* EDITOR_LOG_PANE_HPP_ */
