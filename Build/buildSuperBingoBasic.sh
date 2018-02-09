cd ../
SOURCE_ROOT=`pwd`

# cache old revision
PROJECT_OLD_VERSION=`git rev-list --count HEAD`

# Get the revision number for the build
PROJECT_VERSION=`git rev-list --count HEAD`

# Calculate number of changes since last build, and request git commit logs
# COMMENTING OUT FOR NOW BECAUSE THERE'S AN ISSUE WITH IT.
PROJECT_NUM_COMMITS=`expr $PROJECT_VERSION - $PROJECT_OLD_VERSION`

clear
cd ./Project/AssetServer/Projects/GamingArts/Bingo/

# Needed for code-sign attempting to run remotely from Windows build machine through SSH...
security unlock-keychain -p Gaming.1 /Users/Deployment/Library/Keychains/login.keychain
# security set-keychain-settings -t 6400 -l ~/Library/Keychains/login.keychain

# Remove the old deployments folder, as our deployment will be creating a new one
rm -rf "${SOURCE_ROOT}/Project/AssetServer/Projects/GamingArts/Bingo/deployments"

# Build the ARM and AARCH64 builds of the project
/Applications/Marmalade.app/Contents/s3e/bin/mkb "${SOURCE_ROOT}/Project/AssetServer/Projects/GamingArts/Bingo/SuperBingo.mkb" --release --arm --compiler=gcc --no-ide --make --cflags=-std=c++11
##/Applications/Marmalade.app/Contents/s3e/bin/mkb "${SOURCE_ROOT}/Project/AssetServer/Projects/GamingArts/Bingo/SuperBingo.mkb" --release --aarch64 --compiler=gcc --no-ide --make --cflags=-std=c++11

# Deploy the build and copy it over to the /Builds directory for later
/Applications/Marmalade.app/Contents/s3e/bin/s3e_deploy "${SOURCE_ROOT}/Project/AssetServer/Projects/GamingArts/Bingo/build_superbingo_xcode/deploy_config.py" -n --config iOS --os iphone --arch arm --gcc --release-loader
cp "${SOURCE_ROOT}/Project/AssetServer/Projects/GamingArts/Bingo/deployments/iOS/iphone/release/arm/superbingo.ipa" "${SOURCE_ROOT}/Build/IPAs/superbingo.ipa"
cp "${SOURCE_ROOT}/Project/AssetServer/Projects/GamingArts/Bingo/deployments/iOS/iphone/release/arm/superbingo.app.zip" "${SOURCE_ROOT}/Build/ZIPs/superbingo.app.zip"