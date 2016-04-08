/**********************************************************************

  Audacity: A Digital Audio Editor

  WaveTrack.h

  Dominic Mazzoni

**********************************************************************/

#ifndef __AUDACITY_WAVETRACK__
#define __AUDACITY_WAVETRACK__

#include "Track.h"
#include "SampleFormat.h"
#include "WaveClip.h"
#include "Experimental.h"
#include "widgets/ProgressDialog.h"

#include <vector>
#include <wx/gdicmn.h>
#include <wx/longlong.h>
#include <wx/thread.h>

#include "WaveTrackLocation.h"

class SpectrogramSettings;
class WaveformSettings;
class TimeWarper;

//
// Tolerance for merging wave tracks (in seconds)
//
#define WAVETRACK_MERGE_POINT_TOLERANCE 0.01

#ifdef EXPERIMENTAL_OUTPUT_DISPLAY
#define MONO_WAVE_PAN(T) (T != NULL && T->GetChannel() == Track::MonoChannel && T->GetKind() == Track::Wave && ((WaveTrack *)T)->GetPan() != 0 && WaveTrack::mMonoAsVirtualStereo && ((WaveTrack *)T)->GetDisplay() == WaveTrack::WaveformDisplay)

#define MONO_PAN  (mPan != 0.0 && mChannel == MonoChannel && mDisplay == WaveformDisplay && mMonoAsVirtualStereo)
#endif

/// \brief Structure to hold region of a wavetrack and a comparison function
/// for sortability.
struct Region
{
   Region() : start(0), end(0) {}
   Region(double start_, double end_) : start(start_), end(end_) {}

   double start, end;

   //used for sorting
   bool operator < (const Region &b) const
   {
      return this->start < b.start;
   }
};

class Regions : public std::vector < Region > {};

class Envelope;

class AUDACITY_DLL_API WaveTrack final : public Track {

 private:

   //
   // Constructor / Destructor / Duplicator
   //
   // Private since only factories are allowed to construct WaveTracks
   //

   WaveTrack(DirManager * projDirManager,
             sampleFormat format = (sampleFormat)0,
             double rate = 0);
   WaveTrack(const WaveTrack &orig);

   void Init(const WaveTrack &orig);

   Track::Holder Duplicate() const override;

#ifdef EXPERIMENTAL_OUTPUT_DISPLAY
   void VirtualStereoInit();
#endif
   friend class TrackFactory;

 public:
#ifdef EXPERIMENTAL_OUTPUT_DISPLAY
   static bool mMonoAsVirtualStereo;
#endif

   typedef WaveTrackLocation Location;
   using Holder = std::unique_ptr<WaveTrack>;

   virtual ~WaveTrack();
   double GetOffset() const override;
   void SetOffset(double o) override;

   /** @brief Get the time at which the first clip in the track starts
    *
    * @return time in seconds, or zero if there are no clips in the track
    */
   double GetStartTime() const;

   /** @brief Get the time at which the last clip in the track ends, plus
    * recorded stuff
    *
    * @return time in seconds, or zero if there are no clips in the track.
    */
   double GetEndTime() const;

   //
   // Identifying the type of track
   //

   int GetKind() const override { return Wave; }
#ifdef EXPERIMENTAL_OUTPUT_DISPLAY
   int GetMinimizedHeight() const override;
#endif
   //
   // WaveTrack parameters
   //

   double GetRate() const;
   void SetRate(double newRate);

   // Multiplicative factor.  Only converted to dB for display.
   float GetGain() const;
   void SetGain(float newGain);

   // -1.0 (left) -> 1.0 (right)
   float GetPan() const;
#ifdef EXPERIMENTAL_OUTPUT_DISPLAY
   bool SetPan(float newPan);
#else
   void SetPan(float newPan);
#endif
   // Takes gain and pan into account
   float GetChannelGain(int channel) const;
#ifdef EXPERIMENTAL_OUTPUT_DISPLAY
   void SetVirtualState(bool state, bool half=false);
#endif
   sampleFormat GetSampleFormat() { return mFormat; }
   bool ConvertToSampleFormat(sampleFormat format);

   const SpectrogramSettings &GetSpectrogramSettings() const;
   SpectrogramSettings &GetSpectrogramSettings();
   SpectrogramSettings &GetIndependentSpectrogramSettings();
   void SetSpectrogramSettings(SpectrogramSettings *pSettings);

   const WaveformSettings &GetWaveformSettings() const;
   WaveformSettings &GetWaveformSettings();
   WaveformSettings &GetIndependentWaveformSettings();
   void SetWaveformSettings(WaveformSettings *pSettings);

   //
   // High-level editing
   //

   Track::Holder Cut(double t0, double t1) override;
   Track::Holder Copy(double t0, double t1) const override;
   Track::Holder CopyNonconst(double t0, double t1) /* not override */;
   bool Clear(double t0, double t1) override;
   bool Paste(double t0, const Track *src) override;
   bool ClearAndPaste(double t0, double t1,
                              const Track *src,
                              bool preserve = true,
                              bool merge = true,
                              TimeWarper *effectWarper = NULL) /* not override */;

   bool Silence(double t0, double t1) override;
   bool InsertSilence(double t, double len) override;

   bool SplitAt(double t) /* not override */;
   bool Split(double t0, double t1) /* not override */;
   // Track::Holder CutAndAddCutLine(double t0, double t1) /* not override */;
   bool ClearAndAddCutLine(double t0, double t1) /* not override */;

   Track::Holder SplitCut(double t0, double t1) /* not override */;
   bool SplitDelete(double t0, double t1) /* not override */;
   bool Join(double t0, double t1) /* not override */;
   bool Disjoin(double t0, double t1) /* not override */;

   bool Trim(double t0, double t1) /* not override */;

   bool HandleClear(double t0, double t1, bool addCutLines, bool split);

   bool SyncLockAdjust(double oldT1, double newT1) override;

   /** @brief Returns true if there are no WaveClips in the specified region
    *
    * @return true if no clips in the track overlap the specified time range,
    * false otherwise.
    */
   bool IsEmpty(double t0, double t1);

   /** @brief Append the sample data to the WaveTrack. You must call Flush()
    * after the last Append.
    *
    * If there is an existing WaveClip in the WaveTrack then the data is
    * appended to that clip. If there are no WaveClips in the track, then a NEW
    * one is created.
    */
   bool Append(samplePtr buffer, sampleFormat format,
               sampleCount len, unsigned int stride=1,
               XMLWriter* blockFileLog=NULL);
   /// Flush must be called after last Append
   bool Flush();

   bool AppendAlias(const wxString &fName, sampleCount start,
                    sampleCount len, int channel,bool useOD);

   ///for use with On-Demand decoding of compressed files.
   ///decodeType should be an enum from ODDecodeTask that specifies what
   ///Type of encoded file this is, such as eODFLAC
   //vvv Why not use the ODTypeEnum typedef to enforce that for the parameter?
   bool AppendCoded(const wxString &fName, sampleCount start,
                            sampleCount len, int channel, int decodeType);

   ///gets an int with OD flags so that we can determine which ODTasks should be run on this track after save/open, etc.
   unsigned int GetODFlags();

   ///Deletes all clips' wavecaches.  Careful, This may not be threadsafe.
   void DeleteWaveCaches();

   ///Adds an invalid region to the wavecache so it redraws that portion only.
   void  AddInvalidRegion(sampleCount startSample, sampleCount endSample);

   ///
   /// MM: Now that each wave track can contain multiple clips, we don't
   /// have a continous space of samples anymore, but we simulate it,
   /// because there are alot of places (e.g. effects) using this interface.
   /// This interface makes much sense for modifying samples, but note that
   /// it is not time-accurate, because the "offset" is a double value and
   /// therefore can lie inbetween samples. But as long as you use the
   /// same value for "start" in both calls to "Set" and "Get" it is
   /// guaranteed that the same samples are affected.
   ///
   bool Get(samplePtr buffer, sampleFormat format,
                   sampleCount start, sampleCount len, fillFormat fill=fillZero) const;
   bool Set(samplePtr buffer, sampleFormat format,
                   sampleCount start, sampleCount len);
   void GetEnvelopeValues(double *buffer, int bufferLen,
                         double t0, double tstep) const;
   bool GetMinMax(float *min, float *max,
                  double t0, double t1);
   bool GetRMS(float *rms, double t0, double t1);

   //
   // MM: We now have more than one sequence and envelope per track, so
   // instead of GetSequence() and GetEnvelope() we have the following
   // function which give the sequence and envelope which is under the
   // given X coordinate of the mouse pointer.
   //
   WaveClip* GetClipAtX(int xcoord);
   Sequence* GetSequenceAtX(int xcoord);
   Envelope* GetEnvelopeAtX(int xcoord);
   Envelope* GetActiveEnvelope(void);

   WaveClip* GetClipAtSample(sampleCount sample);

   //
   // Getting information about the track's internal block sizes
   // and alignment for efficiency
   //
   
   sampleCount GetBlockStart(sampleCount t) const;
   sampleCount GetBestBlockSize(sampleCount t) const;
   sampleCount GetMaxBlockSize() const;
   sampleCount GetIdealBlockSize();

   //
   // XMLTagHandler callback methods for loading and saving
   //

   bool HandleXMLTag(const wxChar *tag, const wxChar **attrs) override;
   void HandleXMLEndTag(const wxChar *tag) override;
   XMLTagHandler *HandleXMLChild(const wxChar *tag) override;
   void WriteXML(XMLWriter &xmlFile) override;

   // Returns true if an error occurred while reading from XML
   bool GetErrorOpening() override;

   //
   // Lock and unlock the track: you must lock the track before
   // doing a copy and paste between projects.
   //

   bool Lock();
   bool CloseLock(); //similar to Lock but should be called when the project closes.
   bool Unlock();

   /** @brief Convert correctly between an (absolute) time in seconds and a number of samples.
    *
    * This method will not give the correct results if used on a relative time (difference of two
    * times). Each absolute time must be converted and the numbers of samples differenced:
    *    sampleCount start = track->TimeToLongSamples(t0);
    *    sampleCount end = track->TimeToLongSamples(t1);
    *    sampleCount len = (sampleCount)(end - start);
    * NOT the likes of:
    *    sampleCount len = track->TimeToLongSamples(t1 - t0);
    * See also WaveTrack::TimeToLongSamples().
    * @param t0 The time (floating point seconds) to convert
    * @return The number of samples from the start of the track which lie before the given time.
    */
   sampleCount TimeToLongSamples(double t0) const;
   /** @brief Convert correctly between an number of samples and an (absolute) time in seconds.
    *
    * @param pos The time number of samples from the start of the track to convert.
    * @return The time in seconds.
    */
   double LongSamplesToTime(sampleCount pos) const;

   // Get access to the clips in the tracks. This is used by
   // track artists and also by TrackPanel when sliding...it would
   // be cleaner if this could be removed, though...
   WaveClipList::compatibility_iterator GetClipIterator() { return mClips.GetFirst(); }

   // Create NEW clip and add it to this track. Returns a pointer
   // to the newly created clip.
   WaveClip* CreateClip();

   /** @brief Get access to the most recently added clip, or create a clip,
   *  if there is not already one.  THIS IS NOT NECESSARILY RIGHTMOST.
   *
   *  @return a pointer to the most recently added WaveClip
   */
   WaveClip* NewestOrNewClip();

   /** @brief Get access to the last (rightmost) clip, or create a clip,
   *  if there is not already one.
   *
   *  @return a pointer to a WaveClip at the end of the track
   */
   WaveClip* RightmostOrNewClip();

   // Get the linear index of a given clip (-1 if the clip is not found)
   int GetClipIndex(WaveClip* clip);

   // Get the nth clip in this WaveTrack (will return NULL if not found).
   // Use this only in special cases (like getting the linked clip), because
   // it is much slower than GetClipIterator().
   WaveClip* GetClipByIndex(int index);

   // Get number of clips in this WaveTrack
   int GetNumClips() const;

   // Add all wave clips to the given array 'clips' and sort the array by
   // clip start time. The array is emptied prior to adding the clips.
   void FillSortedClipArray(WaveClipArray& clips) const;

   // Before calling 'Offset' on a clip, use this function to see if the
   // offsetting is allowed with respect to the other clips in this track.
   // This function can optionally return the amount that is allowed for offsetting
   // in this direction maximally.
   bool CanOffsetClip(WaveClip* clip, double amount, double *allowedAmount=NULL);

   // Before moving a clip into a track (or inserting a clip), use this
   // function to see if the times are valid (i.e. don't overlap with
   // existing clips).
   bool CanInsertClip(WaveClip* clip);

   // Move a clip into a NEW track. This will remove the clip
   // in this cliplist and add it to the cliplist of the
   // other track (if that is not NULL). No fancy additional stuff is done.
   // unused   void MoveClipToTrack(int clipIndex, WaveTrack* dest);
   void MoveClipToTrack(WaveClip *clip, WaveTrack* dest);

   // Remove the clip from the track and return a pointer to it.
   WaveClip* RemoveAndReturnClip(WaveClip* clip);

   // Append a clip to the track
   void AddClip(WaveClip* clip);

   // Merge two clips, that is append data from clip2 to clip1,
   // then remove clip2 from track.
   // clipidx1 and clipidx2 are indices into the clip list.
   bool MergeClips(int clipidx1, int clipidx2);

   // Cache special locations (e.g. cut lines) for later speedy access
   void UpdateLocationsCache() const;

   // Get cached locations
   const std::vector<Location> &GetCachedLocations() const { return mDisplayLocationsCache; }

   // Expand cut line (that is, re-insert audio, then DELETE audio saved in cut line)
   bool ExpandCutLine(double cutLinePosition, double* cutlineStart = NULL, double* cutlineEnd = NULL);

   // Remove cut line, without expanding the audio in it
   bool RemoveCutLine(double cutLinePosition);

   // This track has been merged into a stereo track.  Copy shared parameters
   // from the NEW partner.
   void Merge(const Track &orig) override;

   // Resample track (i.e. all clips in the track)
   bool Resample(int rate, ProgressDialog *progress = NULL);

   //
   // AutoSave related
   //
   // Retrieve the unique autosave ID
   int GetAutoSaveIdent();
   // Set the unique autosave ID
   void SetAutoSaveIdent(int id);

   //
   // The following code will eventually become part of a GUIWaveTrack
   // and will be taken out of the WaveTrack class:
   //

   enum WaveTrackDisplay {

      // DO NOT REORDER OLD VALUES!  Replace obsoletes with placeholders.

      Waveform = 0,
      MinDisplay = Waveform,

      obsoleteWaveformDBDisplay,

      Spectrum,

      obsolete1, // was SpectrumLogDisplay
      obsolete2, // was SpectralSelectionDisplay
      obsolete3, // was SpectralSelectionLogDisplay
      obsolete4, // was PitchDisplay

      // Add values here, and update MaxDisplay.

      MaxDisplay = Spectrum,

      NoDisplay,            // Preview track has no display
   };

   // Read appropriate value from preferences
   static WaveTrackDisplay FindDefaultViewMode();

   // Handle remapping of enum values from 2.1.0 and earlier
   static WaveTrackDisplay ConvertLegacyDisplayValue(int oldValue);

   // Handle restriction of range of values of the enum from future versions
   static WaveTrackDisplay ValidateWaveTrackDisplay(WaveTrackDisplay display);

   int GetLastScaleType() const { return mLastScaleType; }
   void SetLastScaleType() const;

   int GetLastdBRange() const { return mLastdBRange; }
   void SetLastdBRange() const;

   WaveTrackDisplay GetDisplay() const { return mDisplay; }
   void SetDisplay(WaveTrackDisplay display) { mDisplay = display; }

   void GetDisplayBounds(float *min, float *max) const;
   void SetDisplayBounds(float min, float max) const;
   void GetSpectrumBounds(float *min, float *max) const;
   void SetSpectrumBounds(float min, float max) const;


 protected:
   //
   // Protected variables
   //

   WaveClipList mClips;

   sampleFormat  mFormat;
   int           mRate;
   float         mGain;
   float         mPan;


   //
   // Data that should be part of GUIWaveTrack
   // and will be taken out of the WaveTrack class:
   //
   mutable float         mDisplayMin;
   mutable float         mDisplayMax;
   mutable float         mSpectrumMin;
   mutable float         mSpectrumMax;

   WaveTrackDisplay mDisplay;
   mutable int   mLastScaleType; // last scale type choice
   mutable int           mLastdBRange;
   mutable std::vector <Location> mDisplayLocationsCache;

   //
   // Protected methods
   //

 private:

   //
   // Private variables
   //

   wxCriticalSection mFlushCriticalSection;
   wxCriticalSection mAppendCriticalSection;
   double mLegacyProjectFileOffset;
   int mAutoSaveIdent;

   SpectrogramSettings *mpSpectrumSettings;
   WaveformSettings *mpWaveformSettings;
};

// This is meant to be a short-lived object, during whose lifetime,
// the contents of the WaveTrack are known not to change.  It can replace
// repeated calls to WaveTrack::Get() (each of which opens and closes at least
// one block file).
class WaveTrackCache {
public:
   explicit WaveTrackCache(const WaveTrack *pTrack = 0)
      : mPTrack(0)
      , mBufferSize(0)
      , mOverlapBuffer()
      , mNValidBuffers(0)
   {
      SetTrack(pTrack);
   }
   ~WaveTrackCache();

   const WaveTrack *GetTrack() const { return mPTrack; }
   void SetTrack(const WaveTrack *pTrack);

   // Uses fillZero always
   // Returns null on failure
   // Returned pointer may be invalidated if Get is called again
   // Do not DELETE[] the pointer
   constSamplePtr Get(sampleFormat format, sampleCount start, sampleCount len);

private:
   void Free();

   struct Buffer {
      float *data;
      sampleCount start;
      sampleCount len;

      Buffer() : data(0), start(0), len(0) {}
      void Free() { delete[] data; data = 0; start = 0; len = 0; }
      sampleCount end() const { return start + len; }
   };

   const WaveTrack *mPTrack;
   sampleCount mBufferSize;
   Buffer mBuffers[2];
   GrowableSampleBuffer mOverlapBuffer;
   int mNValidBuffers;
};

#endif // __AUDACITY_WAVETRACK__
