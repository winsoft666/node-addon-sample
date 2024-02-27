{
  "targets": [
    {
      "target_name": "node-addson-sample", # ***.node
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ "main.cpp" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      # Precompiled macros
      "defines": [ 
        "NAPI_CPP_EXCEPTIONS",
      ],
      "conditions": [
        [
          # Compile configurations for windows
          "OS == 'win'", {
            "configurations": {
              "Debug": {
                # Precompiled macros
                "defines": [ "DEBUG", "_DEBUG" ],
                "cflags": [ "-g", "-O0" ],
                "conditions": [
                  [
                    "target_arch=='x64'", {
                      "msvs_configuration_platform": "x64",
                    }
                  ],
                ],
                "msvs_settings": {
                  "VCCLCompilerTool": {
                    "RuntimeLibrary": 1, # /MTd
                    "Optimization": 0, # /Od, no optimization
                    "MinimalRebuild": "false",
                    "OmitFramePointers": "false",
                    "BasicRuntimeChecks": 3, # /RTC1
                    "AdditionalOptions": [
                      "/EHsc"
                    ],
                  },
                  "VCLinkerTool": {
                    "LinkIncremental": 2, # Enable incremental linking
                    # Dependency library
                    "AdditionalDependencies": [
                    ],
                  },
                },
                # Include directories
                "include_dirs": [
                ],
              },
              "Release": {
                # Precompiled macros
                "defines": [ "NDEBUG" ],
                "msvs_settings": {
                  "VCCLCompilerTool": {
                    "RuntimeLibrary": 0, # /MT
                    "Optimization": 3, # /Ox, full optimization
                    "FavorSizeOrSpeed": 1, # /Ot, favour speed over size
                    "InlineFunctionExpansion": 2, # /Ob2, inline anything eligible
                    "WholeProgramOptimization": "false", # Dsiable /GL, whole program optimization, needed for LTCG
                    "OmitFramePointers": "true",
                    "EnableFunctionLevelLinking": "true",
                    "EnableIntrinsicFunctions": "true",
                    "RuntimeTypeInfo": "false",
                    "ExceptionHandling": "2", # /EHsc
                    "AdditionalOptions": [
                      "/MP", # compile across multiple CPUs
                    ],
                    "DebugInformationFormat": 3,
                    "AdditionalOptions": [
                    ],
                  },
                  "VCLibrarianTool": {
                    "AdditionalOptions": [
                      "/LTCG", # link time code generation
                    ],
                  },
                  "VCLinkerTool": {
                    "LinkTimeCodeGeneration": 1, # link-time code generation
                    "OptimizeReferences": 2, # /OPT:REF
                    "EnableCOMDATFolding": 2, # /OPT:ICF
                    "LinkIncremental": 1, # disable incremental linking
                    # Dependency library
                    "AdditionalDependencies": [
                    ],
                  },
                },
                # Include directories
                "include_dirs": [
                ],
              }
            }
          },
        ]
      ]
    }
  ]
}
