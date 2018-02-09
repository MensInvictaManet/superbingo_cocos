# Setup
#  1. Install the p12 keys used by the provisioning profiles.
#       Request these keys from andy@z4.com if needed.
# 

BUILD_FOLDER=`pwd`
ANDROID_BUILD_DIR="/Users/${USER}/Dropbox/SuperBingo/Builds/Development/Client/Android"
DESKTOP_BUILD_DIR="/Users/${USER}/Dropbox/SuperBingo/Builds/Development/Client/Desktop"
IOS_BUILD_DIR="/Users/${USER}/Dropbox/SuperBingo/Builds/Development/Client/iOS"
BUILD_NUMBER=`git rev-list --count HEAD`
BUILD_NUMBER="1.${BUILD_NUMBER}"

mkdir -p "$ANDROID_BUILD_DIR/$BUILD_NUMBER"
mkdir -p "$DESKTOP_BUILD_DIR/$BUILD_NUMBER"
mkdir -p "$IOS_BUILD_DIR/$BUILD_NUMBER"

echo "Building SuperBingo version" $BUILD_NUMBER
curl -X POST -H 'Content-type: application/json' --data '{"text":"Building SuperBingo iOS client '$BUILD_NUMBER'","icon_emoji":":godmode:","username":"Client Build System"}' https://hooks.slack.com/services/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

##### change into project folder to build
cd ../

##### Update build string in app
echo "#define SUPERBINGO_VERSION \"${BUILD_NUMBER}\"" > ./Classes/Version.h

# set the build number
/usr/libexec/PlistBuddy -c "Set CFBundleVersion ${BUILD_NUMBER}" ./proj.ios_mac/ios/Info.plist
/usr/libexec/PlistBuddy -c "Set CFBundleShortVersionString ${BUILD_NUMBER}" ./proj.ios_mac/ios/Info.plist

/usr/libexec/PlistBuddy -c "Set CFBundleVersion ${BUILD_NUMBER}" ./proj.ios_mac/mac/Info.plist
/usr/libexec/PlistBuddy -c "Set CFBundleShortVersionString ${BUILD_NUMBER}" ./proj.ios_mac/mac/Info.plist

#### Copy the most recent provisioning profile into Xcode list (the target profile is hardcoded in the project)

# create if no profile exists
mkdir -p ~/Library/MobileDevice/Provisioning\ Profiles/
rm -Rf ~/Library/MobileDevice/Provisioning\ Profiles/*
cp "${BUILD_FOLDER}"/*.mobileprovision ~/Library/MobileDevice/Provisioning\ Profiles/

##### Mac build
# xcodebuild -project `pwd`/proj.ios_mac/superbingo.xcodeproj -configuration "Release" -target "SuperBingo Mac" -jobs 8
# if [ $? -ne 0 ]; then
#     echo "Mac build Failed, trying IOS Build"
# else
#     pushd proj.ios_mac/build/Release
#     zip -r "${DESKTOP_BUILD_DIR}/${BUILD_NUMBER}/SuperBingo Mac.zip" superbingo\ Mac.app
#     zip -r "${DESKTOP_BUILD_DIR}/${BUILD_NUMBER}/SuperBingo Mac.dSYM.zip" superbingo\ Mac.app.dSYM
#     popd
# #
# # Deliver OSX version to HockeyApp
# #
#     echo "Uploading Mac Version"

#     curl \
#     -F "status=2" \
#     -F "notify=1" \
#     -F "notes=New Version ${BUILD_NUMBER}" \
#     -F "notes_type=0" \
#     -F "ipa=@${DESKTOP_BUILD_DIR}/${BUILD_NUMBER}/SuperBingo Mac.zip" \
#     -F "dsym=@${DESKTOP_BUILD_DIR}/${BUILD_NUMBER}/SuperBingo Mac.dSYM.zip" \
#     -H "X-HockeyAppToken: 65d25bab0492487fba71ef2cc537f588" \
#     "https://rink.hockeyapp.net/api/2/apps/15fce3e7efc74041a0474764b73126a2/app_versions/upload"

# fi

# Provisioning profile setup in the xcode project:
PWD=`pwd`
xcodebuild archive -project "${PWD}"/proj.ios_mac/SuperBingo.xcodeproj -scheme "SuperBingo-mobile" -configuration "Release" -archivePath "./Build/SuperBingo iOS.xcarchive" -jobs 8
EXIT_CODE=$?
if [ $EXIT_CODE -ne 0 ]; then
    echo "IOS build Failed, exiting"
    exit $EXIT_CODE
else
    rm -rf "${IOS_BUILD_DIR}/${BUILD_NUMBER}/SuperBingo iOS.ipa"
    xcodebuild -exportArchive -archivePath "./Build/SuperBingo iOS.xcarchive" -exportPath "${IOS_BUILD_DIR}/${BUILD_NUMBER}/SuperBingo iOS.ipa" -exportFormat ipa -exportProvisioningProfile "SuperBingo_Development_Old"
    EXIT_CODE=$?
    if [ $EXIT_CODE -ne 0 ]; then
        echo "iOS archiving failed, exiting"
        exit $EXIT_CODE
    else
        zip -r "${IOS_BUILD_DIR}/${BUILD_NUMBER}/SuperBingo iOS.dSYM.zip" "./Build/SuperBingo iOS.xcarchive/dSYMs"

        exit $EXIT_CODE
    fi
fi
