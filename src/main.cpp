auto constexpr operator""_uz(unsigned long long int n) noexcept -> std::size_t {
    return static_cast<std::size_t>(n);
}

auto main(int argc, char* argv[]) -> int {
    using namespace fmt::literals;
    char constexpr program_name[] = "bfi";

    enum : std::size_t { MEM_BUF_SIZE = 30'720 }; // 30KiB

    if (argc < 2) {
        fmt::print(
            stderr,
            "{prog_name}: Expected source file filename.\n",
            "prog_name"_a = program_name
        );
        return EXIT_FAILURE;
    }

    auto const src_file_name = static_cast<char const*>(argv[1]);

    auto const src_file_des = open(src_file_name, O_RDONLY | O_CLOEXEC);
    if (src_file_des < 0) {
        fmt::print(
            stderr,
            "{prog_name}: file '{filename}' open failed: {errno_msg}.\n",
            "prog_name"_a = program_name,
            "filename"_a = src_file_name,
            "errno_msg"_a = std::strerror(errno)
        );
        return EXIT_FAILURE;
    }

    auto const try_close = [&]() -> int {
        if (close(src_file_des) != 0) {
            fmt::print(
                stderr,
                "{prog_name}: file '{filename}' close failed: {errno_msg}.\n",
                "prog_name"_a = program_name,
                "filename"_a = src_file_name,
                "errno_msg"_a = std::strerror(errno)
            );
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    };

    struct stat src_file_stat;
    if (fstat(src_file_des, &src_file_stat) != 0) {
        fmt::print(
            stderr,
            "{prog_name}: file '{filename}' stat failed: {errno_msg}.\n",
            "prog_name"_a = program_name,
            "filename"_a = src_file_name,
            "errno_msg"_a = std::strerror(errno)
        );
        return try_close();
    }
    auto const src_file_size = src_file_stat.st_size;

    auto const src_file_map = static_cast<char*>(
        mmap(nullptr, src_file_size, PROT_READ, MAP_PRIVATE, src_file_des, 0)
    );
    if (src_file_map == MAP_FAILED) {
        fmt::print(
            stderr,
            "{prog_name}: file '{filename}' mmap failed: {errno_msg}.\n",
            "prog_name"_a = program_name,
            "filename"_a = src_file_name,
            "errno_msg"_a = std::strerror(errno)
        );
        return try_close();
    }
    auto const src_file_map_end =
        static_cast<char const*>(src_file_map + src_file_size);

    auto try_munmap = [&]() -> int {
        if (munmap(src_file_map, src_file_size) != 0) {
            fmt::print(
                stderr,
                "{prog_name}: file '{filename}' munmap failed: {errno_msg}.\n",
                "prog_name"_a = program_name,
                "filename"_a = src_file_name,
                "errno_msg"_a = std::strerror(errno)
            );
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    };

    if (try_close() != EXIT_SUCCESS) {
        return try_munmap();
    }

    auto open_bracket_count = 0_uz;
    auto closed_bracket_count = 0_uz;

    for (
        auto i = static_cast<char const*>(src_file_map);
        i != src_file_map_end;
        ++i
    ) {
        switch (*i) {
        case '[':
            ++open_bracket_count;
            break;
        case ']':
            ++closed_bracket_count;
            break;
        default:
            break;
        }
    }

    if (open_bracket_count != closed_bracket_count) {
        fmt::print(
            stderr,
            "{prog_name}: unequal amount of opening and closing brackets.\n"
                "#[: {open_bracket_count}\n"
                "#]: {closed_bracket_count}\n",
            "prog_name"_a = program_name,
            "open_bracket_count"_a = open_bracket_count,
            "closed_bracket_count"_a = closed_bracket_count
        );
        return try_munmap();
    }

    auto mem_buf = std::array<unsigned char, MEM_BUF_SIZE>();
    auto data_ptr = mem_buf.data();
    auto const mem_buf_begin = mem_buf.begin();
    auto const mem_buf_end = mem_buf.end();
    auto const mem_buf_last = mem_buf_end - 1;

    auto const loop_stack = static_cast<char const**>(
        ::operator new (open_bracket_count * sizeof(char*), std::nothrow)
    );
    if (loop_stack == nullptr) {
        return try_munmap();
    }
    auto loop_stack_ptr = loop_stack;

    for (
        auto src_iter = static_cast<char const*>(src_file_map);
        src_iter != src_file_map_end;
        ++src_iter
    ) {
        switch (*src_iter) {
        case '+':
            if (data_ptr == mem_buf_end) {
                fmt::print(
                    stderr,
                    "{prog_name}: tried incrementing value at invalid memory "
                        "address {addr}.\n",
                    "prog_name"_a = program_name,
                    "addr"_a = fmt::ptr(mem_buf_end)
                );
                ::operator delete(loop_stack);
                return try_munmap();
            }
            ++(*data_ptr);
            break;
        case '-':
            if (data_ptr == mem_buf_end) {
                fmt::print(
                    stderr,
                    "{prog_name}: tried decrementing value at invalid memory "
                        "address {addr}.\n",
                    "prog_name"_a = program_name,
                    "addr"_a = fmt::ptr(mem_buf_end)
                );
                ::operator delete(loop_stack);
                return try_munmap();
            }
            --(*data_ptr);
            break;
        case '>':
            if (data_ptr == mem_buf_last) {
                data_ptr = mem_buf_begin;
            } else {
                ++data_ptr;
            }
            break;
        case '<':
            if (data_ptr == mem_buf_begin) {
                data_ptr = mem_buf_last;
            } else {
                --data_ptr;
            }
            break;
        case '.':
            if (data_ptr == mem_buf_end) {
                fmt::print(
                    stderr,
                    "{prog_name}: tried reading from invalid memory address "
                        "{addr}.\n",
                    "prog_name"_a = program_name,
                    "addr"_a = fmt::ptr(mem_buf_end)
                );
                ::operator delete(loop_stack);
                return try_munmap();
            }
            std::putchar(*data_ptr);
            break;
        case ',':
            if (data_ptr == mem_buf_end) {
                fmt::print(
                    stderr,
                    "{prog_name}: tried writing to invalid memory address "
                        "{addr}.\n",
                    "prog_name"_a = program_name,
                    "addr"_a = fmt::ptr(mem_buf_end)
                );
                ::operator delete(loop_stack);
                return try_munmap();
            }
            if (auto const input = std::getchar(); input != EOF) {
                *data_ptr = input;
            }
            break;
        case '[':
            if (data_ptr == mem_buf_end) {
                fmt::print(
                    stderr,
                    "{prog_name}: tried reading from invalid memory address "
                        "{addr}.\n",
                    "prog_name"_a = program_name,
                    "addr"_a = fmt::ptr(mem_buf_end)
                );
                ::operator delete(loop_stack);
                return try_munmap();
            }
            if (*data_ptr != '\0') {
                *loop_stack_ptr++ = src_iter;
            } else {
                auto skip_loop_counter = 1_uz;
                do {
                    ++src_iter;
                    switch (*src_iter) {
                    case '[':
                        ++skip_loop_counter;
                        break;
                    case ']':
                        --skip_loop_counter;
                        break;
                    }
                } while (skip_loop_counter != 0);
            }
            break;
        case ']':
            if (loop_stack_ptr == loop_stack) {
                fmt::print(
                    stderr,
                    "{prog_name}: unexpected end of loop.\n",
                    "prog_name"_a = program_name
                );
                ::operator delete(loop_stack);
                return try_munmap();
            }
            src_iter = *--loop_stack_ptr - 1;
            break;
        default:
            break;
        }
    }

    ::operator delete(loop_stack);
    return try_munmap();
}
