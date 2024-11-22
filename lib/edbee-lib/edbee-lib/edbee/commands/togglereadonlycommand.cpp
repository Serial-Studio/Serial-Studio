#include "togglereadonlycommand.h"

#include "edbee/texteditorcontroller.h"
#include "edbee/models/changes/mergablechangegroup.h"
#include "edbee/models/texteditorconfig.h"
#include "edbee/models/textdocument.h"
#include "edbee/views/textselection.h"

#include "edbee/debug.h"

namespace edbee {

/// Constructs the toggle readonly command
ToggleReadonlyCommand::ToggleReadonlyCommand()
{

}

void ToggleReadonlyCommand::execute(TextEditorController *controller)
{
    controller->setReadonly(controller->readonly() ? false : true);
}

QString ToggleReadonlyCommand::toString()
{
    return "ToggleReadOnlyCommand()";
}

bool ToggleReadonlyCommand::readonly()
{
    return true;
}

} // edbee
