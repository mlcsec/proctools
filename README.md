# proctools

Small toolkit for extracting information and dumping sensitive strings from Windows processes . Made to accompany another project that's in the works.

<br>

## procsearch.cpp

Dump process memory and search for readable strings. PID, search string, and range supplied only for this version. When the search string is found, the range parameter indicates how many characters each side of the found string should be printed as long as they're valid ASCII.

Compile in VS. Example below dumping user information from Outlook process:
```
PS C:\Users\capde> .\procsearch.exe 7592 "email" 500

[+] Search string "email" FOUND: (...) "age_group": "adult",  "association_status": "{\"com.microsoft.Olk\":\"associated\"}"
,"authority": "https://login.microsoftonline.com/consumers",  "birthday": "1969-3-1",  "display_name": "John Doe",
"email": "capdevuser001@outlook.com",  "first_name": "John",  "id": "3a25f54136ac887c",  "last_name": "Doe",  "location":
"UK",  "login_name":"capdevuser001@outlook.com","onprem_sid": "",  "password_change_url": "",  "phone_number": "" (...)
```

Not functioning correctly with `Inline-Execute-Assembly`. I'm working on resolving this and the C++ BOF version which should allow greater search ranges than the C version currently.

<br>

## procsearch-BOF.c

Compile and load the .cna:
```
x86_64-w64-mingw32-gcc -c procsearch-BOF.c -o procsearch-BOF.o
```

The BOF version is sensitive with buffer sizes so the `<range>` option isn't present and the output is somewhat limited. Feel free to play around with the buffer sizes but be wary you may get `__chkstk` errors or the beacon may hang if too large. 

Ideally wanted to write this in C++ which I've had working locally with bof-vs but running in beacon returns some mangled unknown symbol errors or crashes. Will update when possible. There's probably a better way of implementing search function/buffers in C but this'll work for now.

I wouldn't recommend searching for a single character like `"."` in a very large process as it may hang the beacon. Recommend minimum 3 or 4 character search strings when using BOF. You can really crank the range up with the C++ version.

Example dumping dummy user data from Outlook process:

<br>

https://github.com/mlcsec/proctools/assets/47215311/dccd1223-774e-4dec-8353-91c057900a07

<br>

## procinfo.cs

C# utility to print all file version information for the supplied pid. Compile in VS or with csc:

![procinfo-csharp](https://github.com/mlcsec/proctools/assets/47215311/1db4bff8-d173-4d2e-a972-1e29c2d306a9)

<br>

## procinfo-BOF.c

C BOF that replicates some of the above functionality, need to figure out issues dumping `VerQueryValue(lpVersionInfo, "\\StringFileInfo\\040904b0\\...` values.

Compile and load .cna:
```
x86_64-w64-mingw32-gcc -c procinfo-BOF.c -o procinfo-BOF.o
```

![procinfo-c](https://github.com/mlcsec/proctools/assets/47215311/41e3e9ab-0937-4d48-8b74-87e048c3718b)

<br>

## procargs-BOF.c

Extract command line arguments for the specified process. Compile and load .cna:
```
x86_64-w64-mingw32-gcc -c procargs.c -o procargs-BOF.o
```

![image](https://github.com/mlcsec/proctools/assets/47215311/2d9a7445-ee32-48ae-8894-f8908a63dbc2)


<br>

## prockill-BOF.c

Terminate a process (or just use `kill` in CS, not sure why I made this):
```
x86_64-w64-mingw32-gcc -c prockill-BOF.c -o prockill-BOF.o
```

![prockill-BOF](https://github.com/mlcsec/proctools/assets/47215311/5ee6018b-bb8b-4f42-8372-b9e0f4a0bd6e)
