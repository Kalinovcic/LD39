
String read_all_bytes_from_file(char* path, bool null_terminated)
{
    FILE* file = fopen(path, "rb");
    if (!file)
    {
        critical("Failed to open \"%s\"!\n", path);
        return { NULL, 0 };
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* data = (char*) malloc(size + (null_terminated ? 1 : 0));
    if (!data) critical("Out of memory!\n");
    if (fread(data, size, 1, file) != 1)
    {
        critical("Failed to read from \"%s\"!\n", path);
        return { NULL, 0 };
    }

    if (null_terminated)
        data[size] = 0;

    fclose(file);

    return { data, size };
}

String read_all_bytes_from_file(const char* path, bool null_terminated)
{
    return read_all_bytes_from_file((char*) path, null_terminated);
}
