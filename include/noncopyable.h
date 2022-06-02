#pragma once

namespace demo
{
    class Noncopyable
    {
    protected:
        Noncopyable() = default;
        ~Noncopyable() = default;

    private:
        Noncopyable(const Noncopyable &) = delete;
        void operator=(const Noncopyable &) = delete;
    };
}