Coding Style {#coding-style}
============

The coding style is based on google's styleguide: 

<http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml>

* default names: defaultNames
* class names  : ClassNames
* const names  : CONSTANT_NAMES

Reference, pointer location can be placed next to type typename or to the variable name
* good:   TypeName* object;  TypeName& name;     
* good:   TypeName *object;  TypeName &name; 
* bad:    TypeName * object;  TypeName & name;


Member instances should be private. Subclasses should never access data members directly unless it's stricly necessary.

class members
-------------

A class member name is postfixed with '_'. (To prevent clashes with getter functions)

~~~{.cpp} 
int itemName_;
~~~

A class member pointer that is NOT owned by the classes show be postfixed with "Ref". 
For example a reference to a buffer is added like this

~~~{.cpp} 
TextBuffer* bufferRef_;
~~~

When you have a list of reference the following name is used:

~~~{.cpp} 
QList<Item*> itemRefList_;
~~~


Method definitions
------------------

Separate methods from each other with 2 blank lines.  This improves the visible separation of method bodies.


Ownership
---------

In C / C++ you need to do your own garabage collection. So ownership is important. Here are some rules to make the
ownership in the code clear:

* all class members are 'owned' by the class. If not add the postfix "Ref" (see class members)
* transfering ownership from x => y, prefix the function with the word "give" or "take". 
* object.give( x ) gives the ownership of x to object
* object.take( x ) removes the ownership of x from object

When it is possible for a class to have the ownership of an object optionally, the way to do this is the following. (See redbee::TextEditorController.textDocument[Ref])

* give the class 2 members. For example in the case of a document:
* document_, this is the 'owned' document (only if the document is owner else it is 0)
* documentRef_ , this is the reference to the document. This can be a reference to document_ 
* For working with the document the documentRef_ member is used
* For destruction/giving etc. the document_ member is used

When using lists yout can use the following structure 

~~~{.cpp} 
QList<ClassName*> objectList_ ;    // when owning the items (use qDeleteAll in the destructor) 
QList<ClassName*> objectRefList_;  // use this when you do not own the objectlist
~~~


Memory Leak Detection and include order
---------------------------------------

We use custom memory leak detection. To enable this leak detection it is required to include 'debug.h'
You must include "debug.h" in every Source file AFTER all pre-compiled items and header files. Example:

~~~{.cpp} 
// first you should include the source's header file. (This allows you to see if the headers users 'missing' dependencies'
#include "header-file-of-this-source-file.h"

// then include all Qt headers (and other external libraries) (sorted alpabeticly).  
#include <QHeaders>

// after this include your other header files from your project (alpabeticly is nice!)
#include "other-custom-headers.h"

// then include debug.h so memory leak detection can do it's work
#include "debug.h"
~~~

This implies that you should never use a new or delete keyword in a header file!!!
main.cpp is different, it includes all DEBUG_NEW stuff below ALL existing header files!


Documentation
--------------

We make use of doxygen: <http://www.doxygen.org>

- Put member documentation in the header file. Above the member definition with '/// description'. Or next to the definition '///< description'
- Put class documentation in the header file
- Put method documentation in the source file of the class


Header files
------------
- Use #pragma once.. Most platforms support this keyword, so use it. #ifdef structure is outdated
- Never use code in a header file. Even not for getters and setters. (It has been done in this project on certain places, but recommendation is to not do it, because it can case strange build errors)
- You can make an exception (of course) for template classes / function
- limit the use of templates! This slows down the compilation process and gives very crappy error messages 
- Prefer pointers/references above class inclusions in the header file 


Enumerations
------------
On several places in the code you will find enumeration definitions. We do not aways force the enumeration 
type for variables. The reason for this is, that somethimes it's desirable to extends the number of enumeration options.
And when forcing a enumeration type, you cannot easy pass a custom value and are limited to the predefined definitions.



