echo "Configuring project"
cmake --preset="Debug" -G Ninja -Wno-dev
echo "Project succesfully configured, linking compile commands to cwd"
ln -s "build/Debug/compile_commands.json"
