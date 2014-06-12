flash-to-directx
================

PC only library aimed on easy integration of Adobe Flash Player to DirectX-based applications.


Introduction
================

In order to compile provided source code you need Flash Player to be installed in the system.
Please note that you need to install Flash player for Internet Explorer.
Installation of Flash Player for other browsers is not sufficient (compiler will not find needed interfaces).


FunctionCalls  
================

Function calls between C++ and AS3. 

To make a call of C++ function from AS3 you need to make the following:

In C++ register function with ASInterface::AddCallback(name_for_AS3, class, function);
Call function from AS3: ExternalInterface.call(name_for_AS3, parameters);
To make a call of AS3 function from C++ you need to make the following:

Register AS3 function for external interface: ExternalInterface.addCallback(name_for_C++, AS3_function);
Call function from C++: ASInterface::Call(name_for_C++, parameters);
You can use any number of parameters for AS3 and C++ calls.


KeyboardIssues
=================

There is some known issues with AS2-based Flash movies that uses keyboard (games).

For some unknown yet reason, AS2 function Key.isDown() returns no press in case focus is on application that uses library.
In-depth review shows that Flash calls GetKeyState() while processing WM_TIMER event (is it valid at all? why not GetAsyncKeyState()?).
If host application removes focus from it's window while forwarding WM_TIMER through TranslateMessage()/DispatchMessage() functions, Key.isDown() works as expected.

We've considered this as not an issue for GUI only side of DirectX application and stopped further research on this topic.

