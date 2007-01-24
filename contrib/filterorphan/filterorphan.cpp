#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>

struct ReplaceNode
{
  inline operator const char *(void) const { return Text; }
  const char *Text;
  bool CreatedDir;
  bool CreatedParent;
  bool Used;
};

enum TriggerType
{
  Type_MoveOrphan,
  Type_SuppressCreate,
  Type_OneShotReplace,
  Type_NoMove,
  Type_OneShotDupe,
  Type_MultiReplace,
  Type_MultiDupe,
};

struct TriggerNode
{
  const char *TriggerText;
  const char *CopyText;
  union
  {
    ReplaceNode *Node;
    int          SkipCount;
  };
  TriggerType Type;
};

// increase max number of triggers as needed,
// but NB current implementation cannot cope with DIFFERENT text starting with same letter.
// (same text with different Node is ok though)
TriggerNode Triggers[200];
int TriggerCount=0;

int RevSubst[100000];
int MaxRevSeen=0;

char DupeBuffer[1000000];
int DupeIndex=0;
bool DoneDupePrint;

ReplaceNode ReplaceIndex[26][26][26][26];

struct
{
  const char *ReplaceText;
  const char *NextText;
  const char *NextReplace; // of just the NextText for a non-dupe, or of entire path for dupe.
  bool        Dupe;
  char        SkipCount;
} SpecialReplace[] =
{
  { "Plugins","/Gsc200s/Gsc200s.rc2","/Gsc200s/res/Gsc200s.rc2",false,0 },
// snip other SpecialReplace entries.
};

void Replace(const char *FileID,const char *ReplaceText,bool DirAlreadyExists=true,const char *ReplaceText2=NULL,const char *ReplaceTrigger=NULL,const char *ParentCreate=NULL)
{
  ReplaceNode *Node=&ReplaceIndex[FileID[0]-'A'][FileID[1]-'A'][FileID[2]-'A'][FileID[3]-'A'];
  Node->Text=ReplaceText;
  Node->CreatedDir=DirAlreadyExists;
  Node->CreatedParent=ParentCreate==NULL;
  if(ReplaceText2)
  {
    Triggers[TriggerCount].Node=Node;
    Triggers[TriggerCount].CopyText=ReplaceText2;
    Triggers[TriggerCount].TriggerText=ReplaceTrigger?ReplaceTrigger:ReplaceText2;
    Triggers[TriggerCount].Type=Type_MoveOrphan;
    TriggerCount++;
  }
  if(!DirAlreadyExists && strncmp(ReplaceText,"orphaned",8)!=0)
  {
    if(ParentCreate)
    {
      if(strncmp(ParentCreate,ReplaceText,strlen(ParentCreate))!=0)
        fprintf(stderr,"!!!!!! BAD PARAMETER !!!!!!\n");
      // suppress parent when it gets created
      Triggers[TriggerCount].Node=NULL;
      Triggers[TriggerCount].CopyText=NULL;
      Triggers[TriggerCount].TriggerText=ParentCreate;
      Triggers[TriggerCount].Type=Type_SuppressCreate;
      TriggerCount++;
    }
    // suppress when it does get created.
    Triggers[TriggerCount].Node=NULL;
    Triggers[TriggerCount].CopyText=NULL;
    Triggers[TriggerCount].TriggerText=ReplaceText;
    Triggers[TriggerCount].Type=Type_SuppressCreate;
    TriggerCount++;
  }
}

void OneShotReplace(const char *Trigger,const char *ReplaceText,int SkipCount=0)
{
  Triggers[TriggerCount].SkipCount=SkipCount;
  Triggers[TriggerCount].CopyText=ReplaceText;
  Triggers[TriggerCount].TriggerText=Trigger;
  Triggers[TriggerCount].Type=Type_OneShotReplace;
  TriggerCount++;
}
void OneShotDupe(const char *Trigger,const char *ReplaceText,int SkipCount=0)
{
  Triggers[TriggerCount].SkipCount=SkipCount;
  Triggers[TriggerCount].CopyText=ReplaceText;
  Triggers[TriggerCount].TriggerText=Trigger;
  Triggers[TriggerCount].Type=Type_OneShotDupe;
  TriggerCount++;
}
void MultiReplace(const char *Trigger,const char *ReplaceText,int SkipCount=0)
{
  Triggers[TriggerCount].SkipCount=SkipCount;
  Triggers[TriggerCount].CopyText=ReplaceText;
  Triggers[TriggerCount].TriggerText=Trigger;
  Triggers[TriggerCount].Type=Type_MultiReplace;
  TriggerCount++;
}
void MultiDupe(const char *Trigger,const char *ReplaceText,int SkipCount=0)
{
  Triggers[TriggerCount].SkipCount=SkipCount;
  Triggers[TriggerCount].CopyText=ReplaceText;
  Triggers[TriggerCount].TriggerText=Trigger;
  Triggers[TriggerCount].Type=Type_MultiDupe;
  TriggerCount++;
}
void NoMove(const char *Source,const char *Dest)
{
  Triggers[TriggerCount].Node=NULL;
  Triggers[TriggerCount].CopyText=Source;
  Triggers[TriggerCount].TriggerText=Dest;
  Triggers[TriggerCount].Type=Type_NoMove;
  TriggerCount++;
}

void SetupReplacements(void)
{
  // first parameter to Replace MUST begin with 4 capital letters.
  Replace("AAAAAAAA","MyProject"); // 
  Replace("CAAAAAAA","orphaned/DongleCheck",false); // /DongleCheck.cpp
  Replace("DAAAAAAA","orphaned/DongleCheck"); // /DongleCheck.dsp
  Replace("EAAAAAAA","orphaned/DongleCheck"); // /DongleCheck.h
  Replace("FAAAAAAA","orphaned/DongleCheck"); // /DongleCheck.rc
  Replace("GAAAAAAA","orphaned/DongleCheck"); // /DongleCheckDlg.cpp
  Replace("HAAAAAAA","orphaned/DongleCheck"); // /DongleCheckDlg.h
  Replace("IAAAAAAA","orphaned/DongleCheck"); // /ReadMe.txt
  Replace("JAAAAAAA","orphaned/DongleCheck"); // /resource.h
  Replace("KAAAAAAA","orphaned/DongleCheck"); // /StdAfx.cpp
  Replace("LAAAAAAA","orphaned/DongleCheck"); // /StdAfx.h
  Replace("EKAAAAAA","orphaned/DongleCheck/res",false); // /cross.bmp
  Replace("FKAAAAAA","orphaned/DongleCheck/res"); // /DongleCheck.ico
  Replace("GKAAAAAA","orphaned/DongleCheck/res"); // /DongleCheck.rc2
  Replace("HKAAAAAA","orphaned/DongleCheck/res"); // /tick.bmp
// snip various other Replace(...) lines...
  Replace("CJBAAAAA","Plugin/Gsc200s",true,"Plugins/Gsc200s","Plugins"); // /Gsc200sPlugin.cpp
// snip many more Replace(...) lines...

// special triggers
  NoMove("Plugins/Flyte","Flyte");
  RevSubst[302]=370;
  OneShotReplace("InstallImage/Install/v2.26.ipr","InstallImage/Install/V2.26.ipr");
  OneShotReplace("InstallImage/Install/v2.26.ipr","InstallImage/Install/V2.26.ipr");

// snip various fixups of stuff that failed to get put in the Plugins/Flyte project
// mainly OneShotReplace and OneShotDupe lines, and some Multi... lines.
}


int main(void)
{
  _setmode( _fileno( stdin ), _O_BINARY );
  _setmode( _fileno( stdout ), _O_BINARY );

  memset(ReplaceIndex,0,sizeof(ReplaceIndex));
  memset(RevSubst,0,sizeof(RevSubst));

  SetupReplacements();

  const char *DirCreateText="\n"
  "Node-kind: dir\n"
  "Node-action: add\n"
  "Prop-content-length: 10\n"
  "Content-length: 10\n"
  "\n"
  "PROPS-END\n"
  "\n"
  "\n"
  "Node-path: ";
  int DirCreateTextLen=strlen(DirCreateText);
  const char NodePathTrigger[]="Node-path:";
  const char NodeRevTrigger[]="Node-copyfrom-rev: ";
  const char NextRevTrigger[]="Revision-number:";
  bool NodePathFlag;
  bool LastSpace=false;
  const char Prefix[]="orphaned/_";
  const char *TriggerStart=NULL;
  const char *TriggerPtr=NULL;
  const char *Marker="!!!!!!!this should not appear in the output file!!!!!!!";
  char c;
//  int dbgcount=50;
  while(fread(&c,1,1,stdin)) // horribly slow but simple 1 char at a time...
  {
//    if(dbgcount-->0)
//      fprintf(stderr,"Got char: %d ('%c')\n",c,c);
    if(TriggerPtr && c==*TriggerPtr)
    {
      TriggerPtr++;
      if(!*TriggerPtr)
      {
        if(DupeIndex && TriggerStart!=NodePathTrigger && TriggerStart!=NextRevTrigger && TriggerStart!=NodeRevTrigger)
        {
          fprintf(stderr,"Matched trigger in Dupe mode: %s\n",TriggerStart);
          return 1;
        }
        // complete trigger matched.
        if(TriggerStart==Prefix)
        {
          // matched prefix. next 8 chars are the file ID, followed by the original filename.
          char FileID[9];
          fread(&FileID,1,8,stdin);
          FileID[8]='\0';
          int ValidChars;
          for(ValidChars=0;ValidChars<8;ValidChars++)
          {
  //          fprintf(stderr,"FileID[%d]: %d ('%c')\n",ValidChars,FileID[ValidChars],FileID[ValidChars]);
            if(FileID[ValidChars]<'A' || FileID[ValidChars]>'Z')
              break;
  /*          {
              FileID[8]='\0';
              fprintf(stderr,"Sorry I can't cope with %s%s\n",Prefix,FileID);
              return 1;
              // TODO replace the above with code to ignore the false trigger if necessary.
            }*/
          }
          ReplaceNode *Node=ValidChars<8?NULL:&ReplaceIndex[FileID[0]-'A'][FileID[1]-'A'][FileID[2]-'A'][FileID[3]-'A'];
          const char *ReplaceText=Node?Node->Text:NULL;
          if(ReplaceText)
          {
            if(ReplaceText==Marker)
            {
              fwrite(&Prefix,1,TriggerPtr-Prefix,stdout);
              fwrite(FileID,1,8,stdout);
            }
            else
            {
              if(!Node->CreatedParent)
              {
                // need to create the parent of ReplaceText first.
                int LastSep=0;
                for(int i=0;ReplaceText[i];i++)
                  if(ReplaceText[i]=='/')
                    LastSep=i;
                fwrite(ReplaceText,1,LastSep,stdout);
                fwrite(DirCreateText,1,DirCreateTextLen,stdout);
                Node->CreatedParent=true; // avoid re-creating when it is updated, copied, etc.
              }
              fwrite(ReplaceText,1,strlen(ReplaceText),stdout);

              Node->Used=true;

              // now check if this was a dir creation, and if so if we already created the same
              // replacement dir already. If so skip ahead as next section will always be the
              // node creation within the dir.

              fread(&c,1,1,stdin);
  //            fprintf(stderr,"Next char: %d ('%c')\n",c,c);
              if(c=='\n')
              {
                // this is referring just to the orphaned dir itself. next characters should match
                // the following text:
                int DirCreateMatchLen=1;

                while(DirCreateMatchLen<DirCreateTextLen)
                {
                  fread(&c,1,1,stdin);
                  if(c!=DirCreateText[DirCreateMatchLen++])
                  {
                    fprintf(stderr,"DirCreateText Mismatch at character %d (got '%c' instead of '%c'):%s\n",
                            DirCreateMatchLen,c,DirCreateText[DirCreateMatchLen-1],DirCreateText);
                    // don't modify output, so write the stuff we read.
                    fwrite(DirCreateText,1,DirCreateMatchLen-1,stdout);
                    fwrite(&c,1,1,stdout);
                    break;
                  }
                }
                if(!(DirCreateMatchLen<DirCreateTextLen))
                {
                  char LabelPathText[256];
                  char LabelPathTextLen=0;
                  char PathTriggerText[64]="orphaned/_";
                  char PathTriggerTextLen=strlen(strncat(PathTriggerText,FileID,8));
                  char PathTriggerMatchLen=0;
                  while(PathTriggerMatchLen<PathTriggerTextLen)
                  {
                    fread(&c,1,1,stdin);
                    if(c==PathTriggerText[PathTriggerMatchLen])
                      PathTriggerMatchLen++;
                    else if(c=='\n' || LabelPathTextLen+PathTriggerTextLen>250)
                    {
                      LabelPathText[LabelPathTextLen]='\0';
                      fprintf(stderr,"PathTriggerText not found after %c chars. Looking for %s in:\n%s",
                              LabelPathTextLen+PathTriggerTextLen,PathTriggerText,LabelPathText);
                      fwrite(DirCreateText,1,DirCreateTextLen,stdout);
                      fwrite(LabelPathText,1,LabelPathTextLen,stdout);
                      fwrite(PathTriggerText,1,PathTriggerMatchLen,stdout);
                      fwrite(&c,1,1,stdout);
                      break;
                    }
                    else
                    {
                      if(PathTriggerMatchLen)
                      {
                        memcpy(LabelPathText+LabelPathTextLen,PathTriggerText,PathTriggerMatchLen);
                        LabelPathTextLen+=PathTriggerMatchLen;
                        PathTriggerMatchLen=0;
                      }
                      LabelPathText[LabelPathTextLen++]=c;
                    }
                  }

                  if(!(DirCreateMatchLen<DirCreateTextLen))
                  {
                    // full text matched... so do we output or not?
                    LabelPathText[LabelPathTextLen]='\0';
                    if(!Node->CreatedDir)
                    {
                      // write out DirCreateText, but replacing the last section with ReplaceText...
                      fwrite(DirCreateText,1,DirCreateTextLen,stdout);
                      fwrite(LabelPathText,1,LabelPathTextLen,stdout);
                      fwrite(ReplaceText,1,strlen(ReplaceText),stdout);
//                      Node->CreatedDir=true;
//                      fprintf(stderr,"DirCreateText Matched and output! Label:'%s'\n",LabelPathText);
                    }
//                    else
//                      fprintf(stderr,"DirCreateText Matched and suppressed label:'%s'\n",LabelPathText);
                  }
                }
              }
              else
              {
                int Index;
                char Buffer[100];
                int Count=0;
                Buffer[Count++]=c;
                for(Index=0;Index<sizeof(SpecialReplace)/sizeof(*SpecialReplace) && Count;Index++)
                {
                  if(strcmp(SpecialReplace[Index].ReplaceText,ReplaceText)==0)
                  {
                    if(strncmp(SpecialReplace[Index].NextText,Buffer,Count)==0)
                    {
                      for(;;)
                      {
                        if(SpecialReplace[Index].NextText[Count]=='\0')
                        {
                          if(SpecialReplace[Index].SkipCount)
                            SpecialReplace[Index].SkipCount--;
                          else
                          {
//                            fprintf(stderr,"%s -> %s ",SpecialReplace[Index].NextText,SpecialReplace[Index].NextReplace);
                            if(SpecialReplace[Index].Dupe)
                            {
                              if(NodePathFlag)
                              {
                                DupeBuffer[0]=' ';
                                DupeIndex=1+strlen(strcpy(&DupeBuffer[1],SpecialReplace[Index].NextReplace));
//                                fprintf(stderr,"[DupeStart]\n");
                              }
//                              else
  //                              fprintf(stderr,"[Ignored]\n");
                            }
                            else
                            {
                              fwrite(SpecialReplace[Index].NextReplace,1,strlen(SpecialReplace[Index].NextReplace),stdout);
                              Count=0;
                            }
                            break;
                          }
                        }
                        fread(&Buffer[Count++],1,1,stdin);
                        if(Buffer[Count-1]!=SpecialReplace[Index].NextText[Count-1])
                          break;
                      }
                    }
                  }
                }

                if(Count)
                  fwrite(Buffer,1,Count,stdout);
//                else
  //                fprintf(stderr,"...Done SpecialReplace!\n");
              }
            }
          }
          else
          {
            fwrite(&Prefix,1,TriggerPtr-Prefix,stdout);
            fwrite(FileID,1,8,stdout);
            if(ValidChars==8)
            {
              char LeafName[256];
              int LeafLen=0;
              while(LeafLen<sizeof(LeafName)-1)
              {
                fread(&LeafName[LeafLen++],1,1,stdin);
                if(LeafName[LeafLen-1]=='\n')
                  break;
              }
              LeafName[LeafLen]='\0';
              fwrite(LeafName,1,LeafLen,stdout);
              if(LeafLen>1)
              {
                fprintf(stderr,"  Replace(\"%s\",\"????\"); // %s",FileID,LeafName);
                Replace(FileID,Marker);
              }
            }
          }
        }
        else if(TriggerStart==NodePathTrigger || TriggerStart==NextRevTrigger)
        {
          if(DupeIndex)
          {
            fwrite(NodePathTrigger,1,strlen(NodePathTrigger),stdout);
            fwrite(DupeBuffer,1,DupeIndex,stdout);
//            fprintf(stderr,"Done Dupe!\n");
            DupeIndex=0;
          }
          fwrite(TriggerStart,1,TriggerPtr-TriggerStart,stdout);
          NodePathFlag=(TriggerStart==NodePathTrigger);
        }
        else if(TriggerStart==NodeRevTrigger)
        {
          if(DupeIndex)
          {
            memcpy(&DupeBuffer[DupeIndex],TriggerStart,TriggerPtr-TriggerStart);
            DupeIndex+=TriggerPtr-TriggerStart;
          }
          fwrite(TriggerStart,1,TriggerPtr-TriggerStart,stdout);
          char Buffer[8];
          int i;
          for(i=0;i<7;i++)
          {
            fread(&Buffer[i],1,1,stdin);
            if(Buffer[i]=='\n')
            {
              i++;
              break;
            }
            else if(Buffer[i]<'0' || Buffer[i]>'9')
            {
              Buffer[i+1]='\0';
              fprintf(stderr,"Non-numeric revision! (starting \"%s\")\n",Buffer);
              return 1;
            }
          }
          int Rev;
          if(sscanf(Buffer,"%d\n",&Rev))
          {
            if(Rev>MaxRevSeen)
              MaxRevSeen=Rev;
            else if(RevSubst[Rev] && RevSubst[Rev]<MaxRevSeen)
            {
              i=sprintf(Buffer,"%d\n",RevSubst[Rev]);
              fprintf(stderr,"RevSubst OK: %d -> %s",Rev,Buffer);
            }
          }
          else
            fprintf(stderr,"sscanf failed!!!\n");
          fwrite(Buffer,1,i,stdout);
          if(DupeIndex)
          {
            memcpy(&DupeBuffer[DupeIndex],Buffer,i);
            DupeIndex+=i;
          }
        }
        else
        {
          int i=0;
          bool ActionTaken=false;
          while(i<TriggerCount)
          {
            // NB strcmp rather than pointer comparison, as there may be multiple matches.
            if(strcmp(TriggerStart,Triggers[i].TriggerText)==0)
            {
              bool RemoveTrigger=true;
              switch(Triggers[i].Type)
              {
              case Type_MoveOrphan:
                if(!ActionTaken)
                {
                  fwrite(TriggerStart,1,TriggerPtr-TriggerStart,stdout);
                  ActionTaken=true;
                }
                Triggers[i].Node->Text=Triggers[i].CopyText;
                break;
              case Type_SuppressCreate:
                if(!ActionTaken)
                {
                  if(Triggers[i].SkipCount)
                  {
                    fwrite(TriggerStart,1,TriggerPtr-TriggerStart,stdout);
                    Triggers[i].SkipCount--;
                    RemoveTrigger=false;
                  }
                  else
                  {
                    fwrite(TriggerStart,1,TriggerPtr-TriggerStart,stdout);
                    fwrite("-suppressed",1,11,stdout);
                  }
                }
                else
                {
                  fprintf(stderr,"ActionTaken before a SuppressCreate\nTriggered on '%s', type %d\n",Triggers[i].TriggerText,Triggers[i].Type);
                  return 1;
                }
                break;
              case Type_MultiReplace:
                RemoveTrigger=false;
                // follow through
              case Type_OneShotReplace:
                if(!ActionTaken)
                {
                  if(Triggers[i].SkipCount)
                  {
                    fwrite(TriggerStart,1,TriggerPtr-TriggerStart,stdout);
                    Triggers[i].SkipCount--;
                    RemoveTrigger=false;
                  }
                  else
                    fwrite(Triggers[i].CopyText,1,strlen(Triggers[i].CopyText),stdout);
                  ActionTaken=true;
                }
                else
                  RemoveTrigger=false; // could be a second "OneShotReplace" stacked up to make a TwoShotReplace, etc...
                break;
              case Type_MultiDupe:
                RemoveTrigger=false;
                // follow through
              case Type_OneShotDupe:
                if(!ActionTaken)
                {
                  if(Triggers[i].SkipCount)
                  {
                    Triggers[i].SkipCount--;
                    RemoveTrigger=false;
                  }
                  else
                  {
                    fprintf(stderr,"OneShotDupe: %s -> %s\n",Triggers[i].TriggerText,Triggers[i].CopyText);
                    DupeBuffer[0]=' ';
                    DupeIndex=1+strlen(strcpy(&DupeBuffer[1],Triggers[i].CopyText));
                  }
                  fwrite(TriggerStart,1,TriggerPtr-TriggerStart,stdout);
                  ActionTaken=true;
                }
                else
                  RemoveTrigger=false; // could be a second "OneShotDupe" stacked up to make a TwoShotDupe, etc...
                break;
              case Type_NoMove:
                // prevent a 'move' from occuring... do this by creating a OneShotReplace so that the
                // delete action applies to the intended target dir rather than the source dir.
                // this means that the copy still occurs, as does the delete, but delete has a different target.
                if(!ActionTaken)
                {
                  fwrite(TriggerStart,1,TriggerPtr-TriggerStart,stdout);
                  ActionTaken=true;
                }
                Triggers[i].Node=NULL;
                {
                  register const char *Temp=Triggers[i].CopyText;
                  Triggers[i].CopyText=Triggers[i].TriggerText;
                  Triggers[i].TriggerText=Temp;
                }
                Triggers[i].Type=Type_OneShotReplace;
//                TriggerCount++;
                RemoveTrigger=false;
                break;
              default:
                fprintf(stderr,"SOMEONE FORGOT TO WRITE THIS BIT!!!!\nTriggered on '%s', type %d\n",Triggers[i].TriggerText,Triggers[i].Type);
                return 1;
              }
              if(!RemoveTrigger)
                i++;
              else if(i<--TriggerCount)
                memmove(&Triggers[i],&Triggers[i+1],sizeof(*Triggers)*(TriggerCount-i));
            }
            else
              i++;
          }
        }
        TriggerPtr=TriggerStart=NULL;
      }
    }
    else
    {
      if(TriggerPtr!=TriggerStart)
      {
        int CurrentOffset=TriggerPtr-TriggerStart;
        // look for an alternative trigger starting with the same letter(s)
        if(strncmp(TriggerStart,NodeRevTrigger,CurrentOffset)==0 &&
           c==NodeRevTrigger[CurrentOffset])
        {
          TriggerStart=NodeRevTrigger;
          TriggerPtr=TriggerStart+CurrentOffset;
        }
        else if(NodePathFlag)
        {
          for(int i=0;i<TriggerCount;i++)
          {
            if(strncmp(TriggerStart,Triggers[i].TriggerText,CurrentOffset)==0 &&
               c==Triggers[i].TriggerText[CurrentOffset] && (Triggers[i].Type!=Type_MoveOrphan || Triggers[i].Node->Used))
            {
              TriggerStart=Triggers[i].TriggerText;
              TriggerPtr=TriggerStart+CurrentOffset;
              break;
            }
          }
        }
        if(c!=*TriggerPtr)
        {
          fwrite(TriggerStart,1,TriggerPtr-TriggerStart,stdout);
          if(DupeIndex)
          {
            memcpy(&DupeBuffer[DupeIndex],TriggerStart,TriggerPtr-TriggerStart);
            DupeIndex+=TriggerPtr-TriggerStart;
          }
          TriggerPtr=TriggerStart=NULL;
          NodePathFlag=false;
        }
      }
      if(!TriggerStart)
      {
        if(LastSpace && c==Prefix[0])
          TriggerPtr=TriggerStart=Prefix;
        else if(c==NodePathTrigger[0])
          TriggerPtr=TriggerStart=NodePathTrigger;
        else if(c==NextRevTrigger[0])
          TriggerPtr=TriggerStart=NextRevTrigger;
        else if(NodePathFlag)
        {
          for(int i=0;i<TriggerCount;i++)
            if(c==Triggers[i].TriggerText[0] && (Triggers[i].Type!=Type_MoveOrphan || Triggers[i].Node->Used))
            {
              TriggerPtr=TriggerStart=Triggers[i].TriggerText;
              break;
            }
        }
      }
      if(TriggerPtr && c==*TriggerPtr)
        TriggerPtr++;
      else
      {
        fwrite(&c,1,1,stdout);
        if(DupeIndex)
          DupeBuffer[DupeIndex++]=c;
        LastSpace=c==' ';
        if(!LastSpace)
          NodePathFlag=false;
      }
    }
  }
  while(TriggerCount-->0)
  {
    fprintf(stderr,"Unused Trigger: \"%s\" (Type %d, Copy '%s' to %p)\n",
            Triggers[TriggerCount].TriggerText,Triggers[TriggerCount].Type,Triggers[TriggerCount].CopyText,Triggers[TriggerCount].Node);
  }
  return 0;
}
