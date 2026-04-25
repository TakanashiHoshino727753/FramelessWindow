%{Cpp:LicenseTemplate}\
@if '%{Cpp:PragmaOnce}'
#pragma once
@else
#ifndef %{GUARD}
#define %{GUARD}
@endif

@if %{GenerateForm} && ! %{JS: QtSupport.uiAsPointer() }
#include "%{UiHdrFileName}"

@endif

%{JS: (value('BaseClass') === 'FramelessWindow') ? '#include "FramelessWindow.h"' : QtSupport.qtIncludes([ 'QtGui/' + value('BaseClass') ], [ 'QtWidgets/' + value('BaseClass') ]) }

%{JS: Cpp.openNamespaces(value('Class'))}\
@if %{GenerateForm} && %{JS: QtSupport.uiAsPointer() }

@if ! %{JS: Cpp.hasNamespaces(value('Class'))}
QT_BEGIN_NAMESPACE
@endif
namespace Ui { class %{CN}; }
@if ! %{JS: Cpp.hasNamespaces(value('Class'))}
QT_END_NAMESPACE
@endif
@endif

class %{CN} : public %{BaseClass}\
@if %{GenerateForm} && %{JS: QtSupport.uiAsInheritance() }
, private Ui::%{CN}
@else

@endif
{
    Q_OBJECT

public:
    explicit %{CN}(QWidget *parent = nullptr);
    ~%{CN}() override;
@if %{GenerateForm}
@if %{JS: QtSupport.uiAsPointer() }

private:
    Ui::%{CN} *ui;
@elsif %{JS: QtSupport.uiAsMember() }

private:
    Ui::%{CN} ui;
@endif
@endif
};
%{JS: Cpp.closeNamespaces(value('Class'))}\
@if ! '%{Cpp:PragmaOnce}'
#endif // %{GUARD}
@endif