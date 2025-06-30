#pragma once

struct Never
{
    Never() = delete;
    Never(Never const&) = delete;
    void operator=(Never const&) = delete;
};