#pragma once

#include "MathLib.h"
#include <string>

namespace Bat
{
	using SoundSource_t = uintptr_t;

	class ISound;
	class ISoundEngine;
	class IRecordingEngine;

	struct AudioDevice
	{
		std::string id;
		std::string description;
	};

	enum class SampleFormat
	{
		U8,
		S16
	};

	struct AudioFormat
	{
		// channels, 1 for mono, 2 for stereo
		int channel_count; 

		// amount of frames in the sample data or stream
		// If the stream has an unknown length, this is -1
		int frame_count;		

		// samples per second
		int sample_rate;

		// format of the sample data
		SampleFormat sample_format;

		// returns the size of a sample of the data described by the stream data in bytes
		inline int GetSampleSize() const
		{
			return (sample_format == SampleFormat::U8) ? 1 : 2;
		}

		// returns the frame size of the stream data in bytes
		inline int GetFrameSize() const
		{
			return channel_count * GetSampleSize();
		}

		// returns the size of the sample data in bytes
		// Returns an invalid negative value when the stream has an unknown length
		inline int GetSampleDataSize() const
		{
			return GetFrameSize() * frame_count;
		}

		// returns amount of bytes per second
		inline int getBytesPerSecond() const
		{
			return GetFrameSize() * sample_rate;
		}
	};

	enum SoundPlaybackFlags
	{
		SOUND_LOOP = (1 << 0),                 // sound will loop back to start once finished
		SOUND_START_PAUSED = (1 << 1),         // sound starts in paused state
		SOUND_TRACK = (1 << 2),                // if enabled the sound will be tracked via pointer and can be modified mid-playback
		SOUND_ENABLE_SOUND_EFFECTS = (1 << 3), // allows you to use sound effects, only enable this when necessary
	};

	class Audio
	{
	public:
		static std::vector<AudioDevice> GetSoundDeviceList();
		static ISoundEngine* CreateSoundPlaybackDevice();
		static ISoundEngine* CreateSoundPlaybackDevice( const AudioDevice& device );

		static std::vector<AudioDevice> GetRecordingDeviceList();
		static IRecordingEngine* CreateRecordingDevice( const ISoundEngine* pEngine );
		static IRecordingEngine* CreateRecordingDevice( const ISoundEngine* pEngine, const AudioDevice& device );
	};

	class ISound
	{
	public:
		virtual ~ISound() = default;

		virtual bool IsFinished() const = 0;

		virtual bool IsPaused() const = 0;
		virtual void SetPaused( bool paused = true ) = 0;

		virtual void Stop() = 0;

		virtual void SetVolume( float volume ) = 0;
		virtual float GetVolume() const = 0;

		// Sets the pan of the sound, takes values in range [-1.0f, 1.0f]
		virtual void SetPan( float pan ) = 0;
		// Gets the pan of the sound, values returned in range [-1.0f, 1.0f]
		virtual float GetPan() const = 0;

		// Sets whether the sound should loop once finished
		virtual void SetLooped( bool looped ) = 0;
		// Gets whether the sound should loop once finished
		virtual bool IsLooped() const = 0;

		// Sets the current world position of the sound in 3d space
		// Only valid for sounds played using ISoundEngine::Play3D
		virtual void SetWorldPosition( const Vec3& pos ) = 0;
		// Gets the current world position of the sound in 3d space
		// Only valid for sounds played using ISoundEngine::Play3D
		virtual Vec3 GetWorldPosition() const = 0;

		// Sets the current world velocity of the sound in 3d space
		// Needed for the doppler effect
		virtual void SetVelocity( const Vec3& vel ) = 0;
		// Gets the current world velocity of the sound in 3d space
		// Needed for the doppler effect
		virtual Vec3 GetVelocity() const = 0;

		// Gets total sound playback length in milliseconds
		virtual int GetPlaybackLength() const = 0;
		// Sets current sound playback position in milliseconds
		virtual bool SetPlaybackPosition( int pos ) = 0;
		// Gets current sound playback position in milliseconds
		virtual int GetPlaybackPosition() const = 0;
		// Sets sound playback speed (default = 1.0f)
		virtual bool SetPlaybackSpeed( float speed ) = 0;
		// Gets the current sound playback speed (default = 1.0f)
		virtual float GetPlaybackSpeed() const = 0;
	};

	class ISoundEngine
	{
	public:
		virtual ~ISoundEngine() = default;

		// Loads the file (if not already loaded) with specified name and plays it
		// Returns a ISound* if start paused, track, or enable effects flags are enabled
		// NOTE: If an ISound* is returned it must be deleted
		virtual ISound* Play( const std::string& sndname, int flags = 0 ) = 0;
		// Plays the sound from the specified source
		// Returns a ISound* if start paused, track, or enable effects flags are enabled
		// NOTE: If an ISound* is returned it must be deleted
		virtual ISound* Play( const SoundSource_t snd, int flags = 0 ) = 0;
		// Loads the file (if not already loaded) with specified name and plays it as a 3d sound
		// Returns a ISound* if start paused, track, or enable effects flags are enabled
		// NOTE: If an ISound* is returned it must be deleted
		virtual ISound* Play3D( const std::string& sndname, const Vec3& pos, int flags = 0 ) = 0;
		// Plays the sound from the specified source as a 3d soudn
		// Returns a ISound* if start paused, track, or enable effects flags are enabled
		// NOTE: If an ISound* is returned it must be deleted
		virtual ISound* Play3D( const SoundSource_t snd, const Vec3& pos, int flags = 0 ) = 0;

		// Stops all currently playing sounds
		virtual void StopAllSounds() = 0;
		// Pauses or unpauses all currently playing sounds
		virtual void SetAllSoundsPaused( bool paused = true ) = 0;

		virtual SoundSource_t AddSoundFromFile( const std::string& filename ) = 0;
		virtual SoundSource_t AddSoundFromMemory( const char* pData, size_t size ) = 0;
		virtual SoundSource_t AddSoundFromPCMData( const char* pData, size_t size, const AudioFormat& fmt ) = 0;
		// Stops all sounds currently playing from the specified source
		virtual void StopAllSoundForSource( const SoundSource_t snd ) = 0;
		// Removes and frees all resources for specified source
		virtual void RemoveSound( const SoundSource_t snd ) = 0;
		// Removes and frees all resources for all sounds
		virtual void RemoveAllSounds() = 0;

		// Sets the master volume; all sounds played will have their volume multiplied by this
		virtual void SetSoundVolume( float volume ) = 0;
		// Gets the master volume
		virtual float GetSoundVolume() const = 0;

		// Sets the listener world position in 3d space
		// Valid only for 3d sounds
		virtual void SetListenerPosition( const Vec3& pos,
			const Vec3& lookdir,
			const Vec3& vel = { 0.0f, 0.0f, 0.0f },
			const Vec3& up = { 0.0f, 1.0f, 0.0 }
		) = 0;


		// Sound source data

		// Returns whether a sound is currently playing from the specified source
		virtual bool IsCurrentlyPlaying( const SoundSource_t snd ) = 0;
		// Returns the length of the sound in milliseconds
		virtual int GetPlaybackLength( const SoundSource_t snd ) const = 0;
		// Returns the audio format of the sound
		virtual AudioFormat GetAudioFormat( const SoundSource_t snd ) const = 0;
		// Returns whether seeking (setting playback position) is supported for the specified sound source
		virtual bool IsSeekingSupported( const SoundSource_t snd ) const = 0;
		// Sets the default volume for the specified source, all sounds played from this source will have this volume
		// by default
		virtual void SetDefaultVolume( const SoundSource_t snd, float volume ) = 0;
		// Gets the default volume for the specified source
		virtual float GetDefaultVolume( const SoundSource_t snd ) const = 0;
	};

	class IRecordingEngine
	{
	public:
		virtual ~IRecordingEngine() = default;

		// Returns true if we successfully started recording, otherwise false
		virtual bool StartRecording(int sample_rate = 22000,
			SampleFormat sample_format = SampleFormat::S16,
			int channel_count = 1) = 0;
		// Same as default StartRecording but includes a callback
		// Callback gets called with recording audio data chunks
		virtual bool StartRecording(std::function<void(const char* pAudioData, size_t length)> callback,
			int sample_rate = 22000,
			SampleFormat sample_format = SampleFormat::S16,
			int channel_count = 1) = 0;
		virtual void StopRecording() = 0;

		// Clears any existing recording data
		virtual void ClearRecordedAudio() = 0;
		// Returns whether the device is currently recording
		virtual bool IsRecording() const = 0;
		// Gets the raw recording data
		virtual const char* GetRecordingAudioData() const = 0;

		virtual AudioFormat GetAudioFormat() const = 0;
	};
}