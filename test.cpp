#include<iostream>
#include<string>
#include<unistd.h>
using namespace std;
void name(char *a)
{
    int i = 0;
    string n1;
    string n2;
    while (a[i] != ' ')
    {
        i++;
    }
    i = i + 1;
    cout<<i<<endl;
    if (a[i] == '#')
    {
        while (a[i] != ' ')
        {
            n1.push_back(a[i]);
            i++;
        }
        cout<<n1<<endl;
        i = i + 2;
        while (a[i] != '\0')
        {
            n2.push_back(a[i]);
            i++;
        }
        cout<<n2<<endl;
    }
}
int main()
{
    char buff[39]="PRIVMSG #Angel :yes I'm receiving it !";
    char names[50];
    gethostname(names,sizeof(names));
    cout<<names<<endl;
    name(buff);
    cout<<buff<<endl;
    return 0;
}