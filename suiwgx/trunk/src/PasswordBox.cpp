// ======================================================================
//
// Copyright (c) 2008-2012 汪荣, Inc. All rights reserved.
//
// Sharpui库源码遵循CPL协议进行开源，任何个人或团体在此协议上可以自由使用。
//
// ======================================================================

//////////////////////////////////////////////////////////////////////////////
// PasswordBox.cpp

#include <sui/PasswordBox.h>
#include <sui/WindowHelper.h>
#include <suic/render/uirender.h>

namespace ui
{

///////////////////////////////////////////////////////////////////////////////////
// PasswordBox

ImplementTypeofInfo(PasswordBox, suic::Control)

PasswordBox::PasswordBox()
    : _passwordChar('*')
    , _startSel(0)
    , _selCount(0)
    , _caretPos(0)
    , _horizontalOffset(0)
{
    SetClassName(_T("PasswordBox"));

    _caret.SetAutoDelete(false);
}

PasswordBox::~PasswordBox()
{
}

void PasswordBox::Cut()
{
    if (InSelectMode())
    {
        // 先拷贝到内存
        Copy();

        _password.Remove(_startSel, _selCount);

        InvalidateVisual();
        ResetCaretPos();
    }
}

void PasswordBox::Copy()
{
    if (InSelectMode())
    {
        ui::WndHelper helper(this);

        _caret.Hide();

        if (!helper.OpenClipboard())
        {
            return;
        }

        suic::String strSel = _password.Substring(_startSel, _selCount);

        helper.CopyText(strSel);
        helper.CloseClipboard();
    }
}

void PasswordBox::Paste()
{
    ui::WndHelper helper(this);

    if (!helper.OpenClipboard()) 
    {
        return;
    }

    suic::String text;
    helper.PasteText(text);

    _password.Insert(_caretPos, text);

    helper.CloseClipboard();

    InvalidateVisual();
    ResetCaretPos();
}

void PasswordBox::SelectAll()
{
    _caret.Hide();

    _startSel = 0;
    _selCount = _password.Length() - _startSel;
    _caretPos = _selCount;
    InternalSetCaret(0);

    InvalidateVisual();
    ResetCaretPos();
}

suic::String PasswordBox::GetText()
{
    suic::String text;

    return text;
}

void PasswordBox::SetText(const suic::String & text)
{
    _password = text;
    _caretPos = _password.Length();
    InternalSetCaret(0);
}

void PasswordBox::OnRender(suic::DrawingContext * drawing)
{      
    suic::Rect rcdraw(0, 0, RenderSize().cx, RenderSize().cy);

    // 先填充背景
    if (GetBackground())
    {
        GetBackground()->Draw(drawing, &rcdraw);
    }

    // 绘制文本

    suic::Color clrText(ARGB(255,0,0,0));

    if (GetForeground())
    {
        clrText = GetForeground()->ToColor();
    }

    if (_password.Length() > 0)
    {
        suic::FormattedText att;
        suic::Size sizeChar(suic::Render::MeasureCharSize(GetFont()));

        suic::String text(_passwordChar, _password.Length());
        suic::Rect rcCtrl = rcdraw;

        rcCtrl.Deflate(GetBorderThickness());
        rcCtrl.Deflate(GetPadding());
        rcCtrl.left -= _horizontalOffset * sizeChar.cx;

        if (InSelectMode())
        {
            suic::String strDraw;
            int iRight = rcCtrl.right;
            int iStart = 0;
            int iCount = 0;

            GetSelectRange(iStart, iCount);

            if (iStart > 0)
            {
                strDraw = suic::String(_passwordChar, iStart);
                rcCtrl.right = rcCtrl.left + sizeChar.cx * iStart;

                suic::Render::RenderText(drawing, this, strDraw, &rcCtrl, true);
                rcCtrl.left = rcCtrl.right;
            }

            strDraw = suic::String(_passwordChar, iCount);
            rcCtrl.right = rcCtrl.left + sizeChar.cx * iCount;

            att.bkcolor = ARGB(255,100,120,120);

            drawing->DrawText(strDraw.c_str(), strDraw.Length(), &rcCtrl, &att);

            if (iStart + iCount < _password.Length() && rcCtrl.right < iRight)
            {
                iCount = _password.Length() - iStart - iCount;

                strDraw = suic::String(_passwordChar, iCount);
                rcCtrl.left = rcCtrl.right;
                rcCtrl.right += sizeChar.cx * iCount;

                suic::Render::RenderText(drawing, this, strDraw, &rcCtrl, true);
            }
        }
        else
        {
            suic::String strDraw(_passwordChar, _password.Length());
            suic::FormattedText tra;
 
            tra.horzAlign = CoreFlags::Left;
            tra.vertAlign = CoreFlags::Center;
            if (GetFont())
            {
                tra.font = GetFont()->GetFont();
            }
            suic::Render::RenderText(drawing, tra, strDraw, &rcCtrl, true);
        }
    }

    suic::Render::RenderBorder(drawing, this, &rcdraw);
}

bool PasswordBox::InSelectMode() const
{
    return (_selCount != 0);
}

void PasswordBox::CancelSelectMode()
{
    _startSel = 0;
    _selCount = 0;
}

void PasswordBox::DeleteSelection()
{
    if (InSelectMode())
    {
        int iStart = 0;
        int iCount = 0;

        GetSelectRange(iStart, iCount);

        _password.Remove(iStart, iCount);

        if (_selCount > 0)
        {
            _caretPos -= _selCount;
        }

        CancelSelectMode();

        if (_caretPos <= 0)
        {
            _caretPos = 0;
        }

        InternalSetCaret(0);
    }
}

void PasswordBox::GetSelectRange(int& iStart, int& iCount)
{
    iStart = _startSel;
    iCount = _selCount;

    if (_selCount < 0)
    {
        iStart = _startSel + _selCount;
        iCount = -_selCount;
    }
}

void PasswordBox::ResetCaretPos()
{
    _caret.Hide();

    suic::Size size(suic::Render::MeasureCharSize(GetFont()));
    int iCaretPos = (_caretPos - _horizontalOffset) * size.cx + GetBorderThickness().left + GetPadding().left;
    suic::Rect rcCaret(iCaretPos, 0, 1, RenderSize().cy);

    if (rcCaret.left >= RenderSize().cx - GetBorderThickness().right - GetPadding().right)
    {
        rcCaret.left = RenderSize().cx - GetBorderThickness().right - GetPadding().right - 1;
    }

    rcCaret.top = (RenderSize().cy - size.cy) / 2;
    rcCaret.bottom = rcCaret.top + size.cy;
    rcCaret.right = rcCaret.left + _caret.GetWidth();

    _caret.Arrange(rcCaret);
    _caret.Show();
}

int PasswordBox::CalcCaretPos(int xOffset)
{
    suic::Point pt(xOffset, 0);

    xOffset = PointFromScreen(pt).x;
    xOffset -= GetBorderThickness().left;
    xOffset -= GetPadding().left;

    if (xOffset <= 0)
    {
        return 0;
    }

    if (_password.Length() == 0) 
    {
        return 0;
    }

    int iPos = 0;
    suic::Size size(suic::Render::MeasureCharSize(GetFont()));

    iPos = (int)((double)xOffset / (double)size.cx) + _horizontalOffset;

    if (iPos >= (int)_password.Length())
    {
        iPos = (int)_password.Length();
    }

    return iPos;
}

void PasswordBox::InternalSetCaret(int iOff)
{
    // _caretPos为偏移字符数
    _caretPos += iOff;

    suic::Size size(suic::Render::MeasureCharSize(GetFont()));
    int iSize = RenderSize().cx - GetPadding().SumLR() - GetBorderThickness().SumLR();
    // 计算滚动位置
    int iCaretPos = _caretPos * size.cx;

    if (iCaretPos > iSize)
    {
        _horizontalOffset = _caretPos - ((double)iSize / (double)size.cx);
    }
    else if (iCaretPos < _horizontalOffset)
    {
        _horizontalOffset = iCaretPos;
    }
}

void PasswordBox::OnTextInput(suic::KeyboardEventArg& e)
{
    int ch = e.GetKey();
    int code = e.State();

    union wcharfmt
    {
        suic::Char w;
        suic::Byte asc[2];
    };

    static bool bUnicode = false;
    static wcharfmt wf;

    suic::Byte nChar = (suic::Byte)ch;

    // 这是一个双字节的编码
    if (ch >= 0x80)
    {
    }
    // 处理ascii码，注意这里可能是多字节的一部分
    else if (nChar >= 0x80)
    {
        bUnicode = true;
        wf.asc[0] = nChar;
    }
    else
    {
        if (bUnicode)
        {
            wf.asc[1] = nChar;
        }
        else
        {
            // 删除键
            if (VK_BACK == nChar)
            {
                if (InSelectMode())
                {
                    DeleteSelection();
                }
                // 删除一个字符
                else if (_caretPos > 0)
                {
                    _password.Remove(_caretPos - 1, 1);
                    InternalSetCaret(-1);
                }
            }
            else if (nChar >= 32 && nChar <= 126)
            {
                bool bCtrl = (code & MK_CONTROL) == MK_CONTROL;
                
                if (!bCtrl)
                {
                    DeleteSelection();
                    _password.Insert(_caretPos, suic::String(ch, 1));
                    InternalSetCaret(1);
                }
                else if (nChar == 'v' || 
                    nChar == 'V')
                {
                    // paste(粘贴)
                    Paste();
                }
            }
        }

        bUnicode = false;
    }

    InvalidateVisual();
    // 重新设置光标位置
    ResetCaretPos();
}

void PasswordBox::OnKeyDown(suic::KeyboardEventArg& e)
{
    _caret.Hide();

    e.SetHandled(true);

    int state = e.State();
    int ch = e.GetKey();

    bool bShift = (state & MK_SHIFT) == MK_SHIFT;
    bool bCtrl = (state & MK_CONTROL) == MK_CONTROL;

    bool bValid = true;
    bool bUpDown = false;
    bool bSel = InSelectMode();

    if (VK_LEFT == ch)
    {
        CancelSelectMode();

        // 往左移动一个光标
        if (_caretPos > 0)
        {
            InternalSetCaret(-1);
        }
    }
    else if (VK_RIGHT == ch)
    {
        CancelSelectMode();

        // 往右移动一个光标
        if (_caretPos < _password.Length())
        {
            InternalSetCaret(1);
        }
    }
    else if (VK_DELETE == ch)
    {
        if (InSelectMode())
        {
            DeleteSelection();
        }
        else
        {
            // 往右删除一个字符
            _password.Remove(_caretPos, 1);
        }
    }
    else if (VK_ESCAPE == ch)
    {
        // 取消选择模式
        CancelSelectMode();
    }
    else if (VK_HOME == ch)
    {
        // 跳到行首
        _caretPos = 0;
        InternalSetCaret(0);
        CancelSelectMode();
    }
    else if (VK_END == ch)
    {
        // 跳到行末
        _caretPos = _password.Length();
        InternalSetCaret(0);
        CancelSelectMode();
    }
    else if (bCtrl && (ch == 'v' || ch == 'V'))
    {
        //Paste();
        return;
    }
    else if (bCtrl && (ch == 'c' || ch == 'C'))
    {
        //Copy();
        return;
    }
    else if (bCtrl && (ch == 'x' || ch == 'X'))
    {
        //Cut();
        return;
    }
    else if (bCtrl && (ch == 'a' || ch == 'A'))
    {
        SelectAll();
        return;
    }
    else
    {
        ResetCaretPos();
        return;
    }

    // 刷新界面显示

    InvalidateVisual();
    ResetCaretPos();
}

void PasswordBox::OnGotFocus(suic::FocusEventArg& e)
{
    e.SetHandled(true);

    _caret.Hide();

    InvalidateVisual();
    ResetCaretPos();
}

void PasswordBox::OnLostFocus(suic::FocusEventArg& e)
{
    _caret.Hide();

    InvalidateVisual();

    __super::OnLostFocus(e);
}

void PasswordBox::OnMouseLeftButtonDown(suic::MouseEventArg& e)
{
    SetCaptureMouse();

    e.SetHandled(true);
    SetFocus();
    _caret.Hide();

    int code = e.State();

    bool bShift = (code & MK_SHIFT) == MK_SHIFT;
    bool bCtrl = (code & MK_CONTROL) == MK_CONTROL;

    suic::Point tmPt(e.MousePoint());
    bool bValid = false;

    if (bShift)
    {
        // 选择
        bValid = true;
    }
    
    _caretPos = CalcCaretPos(tmPt.x);
    InternalSetCaret(0);

    CancelSelectMode();
    _startSel = _caretPos;

    InvalidateVisual();
    ResetCaretPos();
}

void PasswordBox::OnMouseLeftButtonUp(suic::MouseEventArg& e)
{
    ReleaseCaptureMouse();
}

void PasswordBox::OnMouseLeftButtonDbclk(suic::MouseEventArg& e)
{
    __super::OnMouseLeftButtonDbclk(e);

    _startSel = 0;
    _selCount = _password.Length() - _startSel;
    _caretPos = _selCount;
    InternalSetCaret(0);

    InvalidateVisual();
    ResetCaretPos();

    e.SetHandled(true);
}

void PasswordBox::OnMouseMove(suic::MouseEventArg& e)
{
    e.SetHandled(true);

    if (IsMouseCaptured())
    {
        suic::Point tmPt(e.MousePoint());
        int iPos = CalcCaretPos(tmPt.x);
        int iDif = abs(iPos - _startSel);

        if (iDif > 0)
        {
            _selCount = iPos - _startSel;
            _caretPos = iPos;//_startSel + _selCount;
            InternalSetCaret(0);

            InvalidateVisual();
            ResetCaretPos();
        }
    }
}

void PasswordBox::OnSetCursor(suic::CursorEventArg& e)
{
    suic::Rect rc(0, 0, RenderSize().cx, RenderSize().cy);

    rc.Deflate(GetBorderThickness());
    rc.Deflate(GetPadding());
    rc.Offset(PointToScreen(suic::Point()));

    if (rc.PointIn(e.MousePoint()))
    {
        ::SetCursor(LoadCursor(NULL, IDC_IBEAM));
        e.SetHandled(true);
    }
}

void PasswordBox::OnPropertyChanged(suic::PropertyChangedEventArg& e)
{
    __super::OnPropertyChanged(e);
}

void PasswordBox::OnInitialized()
{
    __super::OnInitialized();

    suic::ObjectPtr obj(GetInternalValue(_T("PasswordChar")));
    
    if (obj && !obj->ToString().Empty())
    {
        _passwordChar = obj->ToString()[0];
    }

    _caret.BeginInit();
    _caret.EndInit();

    AddVisualChild(&_caret);
}

}
