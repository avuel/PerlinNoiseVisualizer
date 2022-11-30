# Walnut App Template

This uses a simple app template from[Walnut](https://github.com/TheCherno/Walnut) - unlike the example within the Walnut repository, this keeps Walnut as an external submodule and is much more sensible for actually building applications. See the [Walnut](https://github.com/TheCherno/Walnut) repository for more details.

## Building and running
Officially supports Windows 10/11 and Visual Studio 2022, with more platforms coming soon. You'll need to have the [Vulkan SDK](https://vulkan.lunarg.com/) installed.

1. Clone recursively: `git clone --recursive https://github.com/avuel/RayTracing/`
2. Run `scripts/Setup.bat`
3. Open `RayTracing.sln` and hit F5 (preferably change configuration to Release or Dist first, Debug is slow)
