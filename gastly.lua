return {
    name = "RaylibEngine",
    version = "0.0.1",
    compiler = "g++",
    flags = "",
    generateCompileCommands = true,
    modules = {
        {
            name = "Copy resources",
            buildCommands = function(runCmd)
                runCmd("cp resources/* build/")
            end
        },
        {
            name = "footbal",
            executable = true,
            libraries = { "raylib" },
            sources = { "src/footballGame.cpp" },
            include = { "include/" },
            output = "build/football"
        },
        {
            name = "physics",
            executable = true,
            libraries = { "raylib", },
            sources = { "src/game.cpp" },
            include = { "include/", },
            output = "build/physics"
        },
        {
            name = "map",
            executable = true,
            libraries = { "raylib", },
            sources = { "src/mapTest.cpp" },
            include = { "include/", },
            output = "build/mapTest"
        },
    },
}
