#ifndef EDITOR_CONFIG_HPP_
#define EDITOR_CONFIG_HPP_

#include "editor_common.hpp"

namespace Editor
{

extern br_token BRT_EDITOR_NUM_CAMERA_PANES_U32;

void RegisterTokens();

struct Config {
    Config() noexcept;

    br_uint_32 num_camera_panes;

    br_error Read(const char *filename) noexcept;
    br_error Write(const char *filename) noexcept;

private:
    template <typename F>
    br_error ReadValue(br_token t, F&& proc) noexcept
    {
        br_error err;
        br_value v;

        if((err = BrAssociativeArrayQuery(this->m_config.get(), t, &v)) != BRE_OK)
            return err;

        proc(&v);
        return BRE_OK;
    }

    br_error WriteValue(br_token t, br_value v) noexcept;
    br_error WriteValue(br_token t, br_uint_32 v) noexcept;

    br_res_ptr<br_associative_array> m_config;
};

} // namespace Editor

#endif /* EDITOR_CONFIG_HPP_ */
