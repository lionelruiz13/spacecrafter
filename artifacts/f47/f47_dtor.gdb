set pagination off
set confirm off
handle SIGUSR1 nostop noprint pass
break BasicMeshLoader::BasicMeshLoader()
break BasicMeshLoader::~BasicMeshLoader()
break ModuleLoaderMgr::~ModuleLoaderMgr()
break ModuleLoaderMgr::init()
info breakpoints
ignore 1 1000000
ignore 2 1000000
ignore 3 1000000
ignore 4 1000000
run
echo \n===== AFTER RUN =====\n
info breakpoints
