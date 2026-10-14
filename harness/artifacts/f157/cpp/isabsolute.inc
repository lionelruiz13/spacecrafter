bool CallSystem::isAbsolute(const std::string path)
{
    return std::filesystem::path(path).is_absolute();
}
