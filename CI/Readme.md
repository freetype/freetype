# Freetype2 CI
 
Continuous integration is a tool used in software development to ensure that your application and/or library builds and runs correctly. Continuous deployment takes this a step further and uses the builds from the CI to create releases for developers and other users to download and use. In this folder are several configuration files and scripts drafted by Greg Williamson for a 2020 GSoC project: https://summerofcode.withgoogle.com/projects/#5724074732421120
 
# How it works
 
Upon each commit, several builds are triggered for various operating systems and build configurations. Each build status is then reported back to the developers in the Pull Request comment section on GitLab or GitHub. Each build creates artifacts containing the newly-built FreeType libraries for developers and users to download. There are also special pipeline phases called "Regression Tests" that run FreeType's built-in demo programs to compare visual outputs between commits. An HTML report that contains a table of the results is uploaded as an artifact for these tests. For text output there is an HTML page displaying a .diff generated and for images a special page is generated that shows both images, layered, so that you can mouse over each to see differences. There is also a generated heatmap image which will draw matching regions of the image in green, and any mismatching regions in red.
 
# Running locally
 
All of the "Regression Tests" can be run locally (Currently only on Linux-based platforms). You will need to have xvfb, imagemagick, and prettydiff installed. You can find more details about this in CI/ft-regression.sh. You will also need "Liberation Fonts" which can be found here: https://releases.pagure.org/liberation-fonts/. The current tests expect the .ttf files to be extracted to ${HOME}/test-fonts. Once set up, you can run all tests by running ./CI/ft-regression.sh <TEST_INDEX>, where TEST_INDEX is the index of the test you wish to run (or "all" to run all tests). Run this command from inside the directory you cloned FreeType to. An HTML report will then be generated to /tmp/ft-test for you to inspect.
 
# Adding / Changing Tests
 
All regular build tests are listed and configured in azure-pipelines.yml and the several templates it includes in the CI/ folder. If you wish to add an additional platform or build configuration, azure-pipelines.yml is the place to do so. For "Regression Tests", there exists a configuration file at CI/ft-tests.config that contains an array where you can add, remove or change test configurations. Each line in the array is a separate test. In order to have Azure run new tests, you will also need to modify azure-pipelines.yml at the root of the project to add/remove tests from the matrix. For each test, you will need to add a line to the yaml like so:
 
```yaml
  strategy:
    matrix:
      Test1:
        TEST: 0
      Test2:
        TEST: 1
      Test3:
        TEST: 2
```
 
# Todo
 
Savannah doesn't allow for integration with any modern CI so this cannot be implemented directly until FreeType moves to GitLab or GitHub. GitHub makes integrating this as simple as a few mouse clicks. For the GitLab route, however, we may need to port my yaml configurations to their CI's format. Another alternative is mirroring the repo. GitLab does have some mirroring capabilities, but for Savannah, developers would need to upload their commits to both manually.
