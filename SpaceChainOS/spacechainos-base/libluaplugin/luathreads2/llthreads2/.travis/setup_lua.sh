# A script for setting up environment for travis-ci testing.
# Sets up Lua and Luarocks.
# LUA must be "lua5.1", "lua5.2" or "luajit".
# PLATFORM must be "linux" or "macosx".

if [ "$LUA" == "luajit" ]; then
  curl http://luajit.org/download/LuaJIT-2.0.2.tar.gz | tar xz
  cd LuaJIT-2.0.2
  make && sudo make install
  cd $TRAVIS_BUILD_DIR;
else
  if [ "$LUA" == "lua5.1" ]; then
    curl http://www.lua.org/ftp/lua-5.1.5.tar.gz | tar xz
    cd lua-5.1.5;
  elif [ "$LUA" == "lua5.2" ]; then
    curl http://www.lua.org/ftp/lua-5.2.3.tar.gz | tar xz
    cd lua-5.2.3;
  fi
  sudo make $PLATFORM install
  cd $TRAVIS_BUILD_DIR;
fi

LUAROCKS_BASE=luarocks-$LUAROCKS_VER
curl http://luarocks.org/releases/$LUAROCKS_BASE.tar.gz | tar xz
cd $LUAROCKS_BASE;

if [ "$LUA" == "luajit" ]; then
  ./configure --lua-suffix=jit --with-lua-include=/usr/local/include/luajit-2.0;
else
  ./configure;
fi

make && sudo make install

cd $TRAVIS_BUILD_DIR