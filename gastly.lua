return {
    name = "RaylibEngine",
    version = "0.0.1",
    compiler = "g++",
    flags = "",
    generateCompileCommands = true,
    modules = {
        {
            name = "Copy resources",
            output = "copyResourcesToBuildDir",
            preBuildCommands= function() return true end,
            buildCommands = function(runCmd)
                return runCmd("cp resources/* build/")
            end,
            executable = true,
        },
        {
            name = "footbal",
            executable = true,
            libraries = { "raylib" },
            sources = { "src/footballGame/" },
            include = { "include/", "include/footballGame/" },
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
