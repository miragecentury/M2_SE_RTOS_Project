//Ride7 calls this function when a project is opened.
//projectpath corresponds to the selected node path

function ProjectOpened(projectpath)
{
   Project.RunTool("Refresh");

   //Insert code to handle the event here
}


//Ride7 calls this function when a project node is selected.
//nodepath corresponds to the selected node path

function NodeSelected(nodepath)
{
   //Insert code to handle the event here
}

//Ride7 calls this function before a build starts.
//apppath corresponds to the application path to be built

function BeforeBuildStart(apppath)
{
   //Shell object
   var shell = new ActiveXObject("WScript.Shell");

   //Search the path for the last separator
   var i = apppath.lastIndexOf('\\');

   //Retrieve project path
   var projectPath = apppath.substring(0, i + 1);

   //Command line
   var execPath = projectPath + "..\\..\\..\\..\\..\\utils\\ResourceCompiler\\bin\\rc.exe";
   var src = projectPath + "..\\resources\\";
   var dest = projectPath + "..\\src\\res.c";

   //Execute resources compiler utility
   shell.Run(execPath + " " + src + " " + dest);
}


//Ride7 calls this function at the end of a build session
//apppath corresponds to the application path that has been built
//errors is 0 when the build has been successfull and 1 otherwise

function AfterBuildFinish(apppath, errors)
{
   //Insert code to handle the event here
}


//Ride7 calls this function before starting a debug session.
//dbipath corresponds to the dbi path containing the information
//about the debug session

function BeforeDebugStart(dbipath)
{
   //Insert code to handle the event here
}


//Ride7 calls this function when a debug session has been terminated.
//dbipath corresponds to the dbi path containing the information
//about the debug session

function AfterDebugFinish(dbipath)
{
   //Insert code to handle the event here
}



//Ride7 calls this function when a breakpoint is hit.
//addr corresponds to the current PC value

function BreakpointHit(addr)
{
   //Insert code to handle the event here
}

//Ride7 calls this function when debugging is stopped.
//addr corresponds to the current PC value

function DebugStop(addr)
{
   //Insert code to handle the event here
}

//Ride7 calls this function when starting a new session.

function IDEStart()
{
   //Insert code to handle the event here
}

//Ride7 calls this function when ending the current session.

function IDEEnd()
{
   //Insert code to handle the event here
}
