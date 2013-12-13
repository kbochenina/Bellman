set currentDir=%CD%
D:
cd \
cd "D:\ITMO\Degree\Programs\Bellman scheme\WfOpt\WFSched\Output"
set /p var="*.jed file name: "
If Not Exist Images md Images
java -jar jedule.jar JeduleStarter -p simgrid -d 1024x768 -o Images\%var%.png -gt png -cm cmap.xml -f %var%.jed
cd /d %currentDir%