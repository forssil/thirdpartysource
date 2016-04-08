/**********************************************************************

  Audacity: A Digital Audio Editor

  PCMAliasBlockFile.h

  Joshua Haberman

**********************************************************************/

#ifndef __AUDACITY_PCMALIASBLOCKFILE__
#define __AUDACITY_PCMALIASBLOCKFILE__

#include "../BlockFile.h"
#include "../DirManager.h"

/// An AliasBlockFile that references uncompressed data in an existing file
class PCMAliasBlockFile /* not final */ : public AliasBlockFile
{
 public:
   /// Constructs a PCMAliasBlockFile, writing the summary to disk
   PCMAliasBlockFile(wxFileName baseFileName,
                     wxFileName aliasedFileName,
                     sampleCount aliasStart,
                     sampleCount aliasLen, int aliasChannel);
   ///Constructs a PCMAliasBlockFile with the option of not writing to disk
   PCMAliasBlockFile(wxFileName fileName,
                     wxFileName aliasedFileName,
                     sampleCount aliasStart,
                     sampleCount aliasLen, int aliasChannel,bool writeSummary);

   PCMAliasBlockFile(wxFileName existingSummaryFileName,
                     wxFileName aliasedFileName,
                     sampleCount aliasStart,
                     sampleCount aliasLen, int aliasChannel,
                     float min, float max, float rms);
   virtual ~PCMAliasBlockFile();

   /// Reads the specified data from the aliased file using libsndfile
   int ReadData(samplePtr data, sampleFormat format,
                        sampleCount start, sampleCount len) override;

   void SaveXML(XMLWriter &xmlFile) override;
   BlockFile *Copy(wxFileName fileName) override;
   void Recover() override;

   static BlockFile *BuildFromXML(DirManager &dm, const wxChar **attrs);
};

#endif

