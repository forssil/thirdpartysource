/**********************************************************************

  Audacity: A Digital Audio Editor

  LabelDialog.h

  Dominic Mazzoni

**********************************************************************/

#ifndef __AUDACITY_LABELDIALOG__
#define __AUDACITY_LABELDIALOG__

#include <vector>
#include <wx/defs.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/grid.h>
#include <wx/string.h>

#include "Internat.h"
#include "widgets/Grid.h"

class TrackFactory;
class TrackList;
class RowData;
class EmptyLabelRenderer;
class LabelTrack;
class ViewInfo;

typedef std::vector<RowData> RowDataArray;

class LabelDialog final : public wxDialog
{
 public:

   LabelDialog(wxWindow *parent,
               TrackFactory &factory,
               TrackList *tracks,
               ViewInfo &viewinfo,
               double rate,
               const wxString & format);
   ~LabelDialog();

    bool Show(bool show = true) override;

 private:

   bool TransferDataToWindow();
   bool TransferDataFromWindow();
   bool Validate();
   void FindAllLabels();
   void AddLabels(LabelTrack *t);
   void FindInitialRow();
   wxString TrackName(int & index, const wxString &dflt = _("Label Track"));

   void OnUpdate(wxCommandEvent &event);
   void OnInsert(wxCommandEvent &event);
   void OnRemove(wxCommandEvent &event);
   void OnImport(wxCommandEvent &event);
   void OnExport(wxCommandEvent &event);
   void OnSelectCell(wxGridEvent &event);
   void OnCellChange(wxGridEvent &event);
   void OnChangeTrack(wxGridEvent &event, int row, RowData *rd);
   void OnChangeLabel(wxGridEvent &event, int row, RowData *rd);
   void OnChangeStime(wxGridEvent &event, int row, RowData *rd);
   void OnChangeEtime(wxGridEvent &event, int row, RowData *rd);
   void OnOK(wxCommandEvent &event);
   void OnCancel(wxCommandEvent &event);

 private:

   Grid *mGrid;
   ChoiceEditor *mChoiceEditor;
   TimeEditor *mTimeEditor;

   RowDataArray mData;

   TrackFactory &mFactory;
   TrackList *mTracks;
   ViewInfo *mViewInfo;
   wxArrayString mTrackNames;
   double mRate;
   wxString mFormat;

   int mInitialRow;

   DECLARE_EVENT_TABLE();
};

#endif
