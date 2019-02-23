#include "PCH.h"
#include "Audio.h"

#include <irrKlang/irrKlang.h>

namespace Bat
{
	static irrklang::ESampleFormat Bat2IrrKlangSampleFormat( SampleFormat format )
	{
		switch( format )
		{
			case SampleFormat::U8:
				return irrklang::ESampleFormat::ESF_U8;
			case SampleFormat::S16:
				return irrklang::ESampleFormat::ESF_S16;
			default:
				ASSERT( false, "Unhandled sample format" );
				return irrklang::ESampleFormat::ESF_U8;
		}
	}

	static SampleFormat IrrKlang2BatSampleFormat( irrklang::ESampleFormat ik_format )
	{
		switch( ik_format )
		{
			case irrklang::ESampleFormat::ESF_U8:
				return SampleFormat::U8;
			case irrklang::ESampleFormat::ESF_S16:
				return SampleFormat::S16;
			default:
				ASSERT( false, "Unhandled irrKlang sample format" );
				return SampleFormat::U8;
		}
	}

	static irrklang::SAudioStreamFormat Bat2IrrKlangAudioFormat( const AudioFormat& format )
	{
		irrklang::SAudioStreamFormat ik_format;
		ik_format.ChannelCount = format.channel_count;
		ik_format.FrameCount = format.frame_count;
		ik_format.SampleRate = format.sample_rate;
		ik_format.SampleFormat = Bat2IrrKlangSampleFormat( format.sample_format );

		return ik_format;
	}

	static AudioFormat IrrKlang2BatAudioFormat( const irrklang::SAudioStreamFormat& ik_format )
	{
		AudioFormat format;
		format.channel_count = ik_format.ChannelCount;
		format.frame_count = ik_format.FrameCount;
		format.sample_rate = ik_format.SampleRate;
		format.sample_format = IrrKlang2BatSampleFormat( ik_format.SampleFormat );

		return format;
	}

	static irrklang::vec3df Bat2IrrKlangVec3f( const Vec3& vec )
	{
		irrklang::vec3df ik_vec;
		ik_vec.X = vec.x;
		ik_vec.Y = vec.y;
		ik_vec.Z = vec.z;

		return ik_vec;
	}

	static Vec3 IrrKlang2BatVec3f( const irrklang::vec3df& ik_vec )
	{
		Vec3 vec;
		vec.x = ik_vec.X;
		vec.y = ik_vec.Y;
		vec.z = ik_vec.Z;

		return vec;
	}

	class IrrKlangSound : public ISound
	{
	public:
		IrrKlangSound( irrklang::ISound* pSound )
			:
			m_pSound( pSound )
		{}
		~IrrKlangSound()
		{
			m_pSound->drop();
		}

		virtual bool IsFinished() const override
		{
			return m_pSound->isFinished();
		}

		virtual bool IsPaused() const override
		{
			return m_pSound->getIsPaused();
		}
		virtual void SetPaused( bool paused ) override
		{
			m_pSound->setIsPaused( paused );
		}

		virtual void Stop() override
		{
			m_pSound->stop();
		}

		virtual void SetVolume( float volume ) override
		{
			m_pSound->setVolume( volume );
		}
		virtual float GetVolume() const override
		{
			return m_pSound->getVolume();
		}

		virtual void SetPan( float pan ) override
		{
			m_pSound->setPan( pan );
		}
		virtual float GetPan() const override
		{
			return m_pSound->getPan();
		}

		virtual void SetLooped( bool looped ) override
		{
			m_pSound->setIsLooped( looped );
		}
		virtual bool IsLooped() const override
		{
			return m_pSound->isLooped();
		}

		virtual void SetWorldPosition( const Vec3& pos ) override
		{
			m_pSound->setPosition( Bat2IrrKlangVec3f( pos ) );
		}
		virtual Vec3 GetWorldPosition() const override
		{
			return IrrKlang2BatVec3f( m_pSound->getPosition() );
		}

		virtual void SetVelocity( const Vec3& vel ) override
		{
			m_pSound->setVelocity( Bat2IrrKlangVec3f( vel ) );
		}
		virtual Vec3 GetVelocity() const override
		{
			return IrrKlang2BatVec3f( m_pSound->getVelocity() );
		}

		virtual int GetPlaybackLength() const override
		{
			return m_pSound->getPlayLength();
		}
		virtual bool SetPlaybackPosition( int pos ) override
		{
			return m_pSound->setPlayPosition( pos );
		}
		virtual int GetPlaybackPosition() const override
		{
			return m_pSound->getPlayPosition();
		}
		virtual bool SetPlaybackSpeed( float speed ) override
		{
			return m_pSound->setPlaybackSpeed( speed );
		}
		virtual float GetPlaybackSpeed() const override
		{
			return m_pSound->getPlaybackSpeed();
		}
	private:
		irrklang::ISound* m_pSound;
	};

	class IrrKlangSoundEngine : public ISoundEngine
	{
	public:
		IrrKlangSoundEngine()
		{
			m_pSoundEngine = irrklang::createIrrKlangDevice();
		}
		IrrKlangSoundEngine( const AudioDevice& device )
		{
			m_pSoundEngine = irrklang::createIrrKlangDevice( irrklang::ESOD_AUTO_DETECT,
				irrklang::ESEO_DEFAULT_OPTIONS,
				device.id.c_str()
			);
		}
		~IrrKlangSoundEngine()
		{
			m_pSoundEngine->drop();
		}

		virtual void Play( const std::string& sndname, bool loop ) override
		{
			irrklang::ISound* ik_sound = m_pSoundEngine->play2D( sndname.c_str(), loop );
		}
		virtual void Play( const SoundSource_t snd, bool loop ) override
		{
			auto ik_snd = ( irrklang::ISoundSource* )snd;
			irrklang::ISound* ik_sound = m_pSoundEngine->play2D( ik_snd, loop );
		}
		virtual ISound* PlayEx( const std::string& sndname, int flags ) override
		{
			bool loop = ( flags & SOUND_LOOP );
			bool paused = ( flags & SOUND_START_PAUSED );
			bool enable_effects = ( flags & SOUND_ENABLE_SOUND_EFFECTS );
			irrklang::ISound* ik_sound = m_pSoundEngine->play2D( sndname.c_str(),
				loop,
				paused,
				true,
				irrklang::ESM_AUTO_DETECT,
				enable_effects
			);

			return new IrrKlangSound( ik_sound );
		}
		virtual ISound* PlayEx( const SoundSource_t snd, int flags ) override
		{
			auto ik_snd = ( irrklang::ISoundSource* )snd;

			bool loop = ( flags & SOUND_LOOP );
			bool paused = ( flags & SOUND_START_PAUSED );
			bool enable_effects = ( flags & SOUND_ENABLE_SOUND_EFFECTS );
			irrklang::ISound* ik_sound = m_pSoundEngine->play2D( ik_snd,
				loop,
				paused,
				true,
				enable_effects
			);

			return new IrrKlangSound( ik_sound );
		}
		virtual void Play3D( const std::string& sndname, const Vec3& pos, bool loop ) override
		{
			irrklang::ISound* ik_sound = m_pSoundEngine->play3D( sndname.c_str(), Bat2IrrKlangVec3f( pos ), loop );
		}
		virtual void Play3D( const SoundSource_t snd, const Vec3& pos, bool loop ) override
		{
			auto ik_snd = ( irrklang::ISoundSource* )snd;
			irrklang::ISound* ik_sound = m_pSoundEngine->play3D( ik_snd, Bat2IrrKlangVec3f( pos ), loop );
		}
		virtual ISound* Play3DEx( const std::string& sndname, const Vec3& pos, int flags ) override
		{
			bool loop = ( flags & SOUND_LOOP );
			bool paused = ( flags & SOUND_START_PAUSED );
			bool enable_effects = ( flags & SOUND_ENABLE_SOUND_EFFECTS );
			irrklang::ISound* ik_sound = m_pSoundEngine->play3D( sndname.c_str(),
				Bat2IrrKlangVec3f( pos ),
				loop,
				paused,
				true,
				irrklang::ESM_AUTO_DETECT,
				enable_effects
			);

			return new IrrKlangSound( ik_sound );
		}
		virtual ISound* Play3DEx( const SoundSource_t snd, const Vec3& pos, int flags ) override
		{
			auto ik_snd = ( irrklang::ISoundSource* )snd;

			bool loop = ( flags & SOUND_LOOP );
			bool paused = ( flags & SOUND_START_PAUSED );
			bool enable_effects = ( flags & SOUND_ENABLE_SOUND_EFFECTS );
			irrklang::ISound* ik_sound = m_pSoundEngine->play3D( ik_snd,
				Bat2IrrKlangVec3f( pos ),
				loop,
				paused,
				true,
				enable_effects
			);

			return new IrrKlangSound( ik_sound );
		}

		virtual void StopAllSounds() override
		{
			m_pSoundEngine->stopAllSounds();
		}
		virtual void SetAllSoundsPaused( bool paused = true ) override
		{
			m_pSoundEngine->setAllSoundsPaused( paused );
		}

		virtual SoundSource_t AddSoundFromFile( const std::string& filename ) override
		{
			return (SoundSource_t)m_pSoundEngine->addSoundSourceFromFile( filename.c_str() );
		}
		virtual SoundSource_t AddSoundFromMemory( const char* pData, size_t size ) override
		{
			return (SoundSource_t)m_pSoundEngine->addSoundSourceFromMemory( const_cast<char*>(pData),
				( irrklang::ik_s32 )size,
				"TODO: what to do here :thinking:"
			);
		}
		virtual SoundSource_t AddSoundFromPCMData( const char* pData, size_t size, const AudioFormat& fmt ) override
		{
			return (SoundSource_t)m_pSoundEngine->addSoundSourceFromPCMData( const_cast<char*>( pData ),
				( irrklang::ik_s32 )size,
				"TODO: here as well",
				Bat2IrrKlangAudioFormat( fmt )
			);
		}
		virtual void StopAllSoundForSource( const SoundSource_t snd ) override
		{
			m_pSoundEngine->stopAllSoundsOfSoundSource( ( irrklang::ISoundSource* )snd );
		}
		virtual void RemoveSound( const SoundSource_t snd ) override
		{
			m_pSoundEngine->removeSoundSource( ( irrklang::ISoundSource* )snd );
		}
		virtual void RemoveAllSounds() override
		{
			m_pSoundEngine->removeAllSoundSources();
		}

		virtual void SetSoundVolume( float volume ) override
		{
			m_pSoundEngine->setSoundVolume( volume );
		}
		virtual float GetSoundVolume() const override
		{
			return m_pSoundEngine->getSoundVolume();
		}

		virtual void SetListenerPosition( const Vec3& pos,
			const Vec3& lookdir,
			const Vec3& vel,
			const Vec3& up ) override
		{
			m_pSoundEngine->setListenerPosition( Bat2IrrKlangVec3f( pos ),
				Bat2IrrKlangVec3f( lookdir ),
				Bat2IrrKlangVec3f( vel ),
				Bat2IrrKlangVec3f( up )
			);
		}

		virtual bool IsCurrentlyPlaying( const SoundSource_t snd ) override
		{
			return m_pSoundEngine->isCurrentlyPlaying( ( irrklang::ISoundSource* )snd );
		}
		virtual int GetPlaybackLength( const SoundSource_t snd ) const override
		{
			auto ik_snd = ( irrklang::ISoundSource* )snd;
			return ik_snd->getPlayLength();
		}
		virtual AudioFormat GetAudioFormat( const SoundSource_t snd ) const override
		{
			auto ik_snd = ( irrklang::ISoundSource* )snd;
			return IrrKlang2BatAudioFormat( ik_snd->getAudioFormat() );
		}
		virtual bool IsSeekingSupported( const SoundSource_t snd ) const override
		{
			auto ik_snd = ( irrklang::ISoundSource* )snd;
			return ik_snd->getIsSeekingSupported();
		}
		virtual void SetDefaultVolume( const SoundSource_t snd, float volume ) override
		{
			auto ik_snd = ( irrklang::ISoundSource* )snd;
			ik_snd->setDefaultVolume( volume );
		}
		virtual float GetDefaultVolume( const SoundSource_t snd ) const override
		{
			auto ik_snd = ( irrklang::ISoundSource* )snd;
			return ik_snd->getDefaultVolume();
		}

		irrklang::ISoundEngine* GetIrrKlangEngine() const { return m_pSoundEngine; }
	private:
		irrklang::ISoundEngine* m_pSoundEngine = nullptr;
	};

	class IrrKlangRecordingEngine : public IRecordingEngine, public irrklang::ICapturedAudioDataReceiver
	{
	public:
		IrrKlangRecordingEngine( const ISoundEngine* pEngine )
		{
			auto pIrrKlangEngine = static_cast<const IrrKlangSoundEngine*>( pEngine )->GetIrrKlangEngine();
			m_pAudioRecorder = irrklang::createIrrKlangAudioRecorder( pIrrKlangEngine );
		}
		IrrKlangRecordingEngine( const ISoundEngine* pEngine, const AudioDevice& device )
		{
			auto pIrrKlangEngine = static_cast<const IrrKlangSoundEngine*>( pEngine )->GetIrrKlangEngine();
			m_pAudioRecorder = irrklang::createIrrKlangAudioRecorder( pIrrKlangEngine,
				irrklang::ESOD_AUTO_DETECT,
				device.id.c_str()
			);
		}
		~IrrKlangRecordingEngine()
		{
			m_pAudioRecorder->drop();
		}

		virtual void OnReceiveAudioDataStreamChunk(unsigned char* audioData, unsigned long lengthInBytes) override
		{
			ASSERT( m_CustomHandledAudioCB, "Invalid callback" );
			m_CustomHandledAudioCB( reinterpret_cast<char*>( audioData ), (size_t)lengthInBytes );
		}

		virtual bool StartRecording(int sample_rate,
			SampleFormat sample_format,
			int channel_count) override
		{
			return m_pAudioRecorder->startRecordingBufferedAudio( sample_rate,
				Bat2IrrKlangSampleFormat( sample_format ),
				channel_count
			);
		}

		virtual bool StartRecording(std::function<void(const char* pAudioData, size_t length)> callback,
			int sample_rate,
			SampleFormat sample_format,
			int channel_count) override
		{
			m_CustomHandledAudioCB = callback;
			return m_pAudioRecorder->startRecordingCustomHandledAudio( this,
				sample_rate,
				Bat2IrrKlangSampleFormat( sample_format ),
				channel_count
			);
		}

		virtual void StopRecording() override
		{
			m_pAudioRecorder->stopRecordingAudio();
		}

		virtual void ClearRecordedAudio() override
		{
			m_pAudioRecorder->clearRecordedAudioDataBuffer();
		}
		virtual bool IsRecording() const override
		{
			return m_pAudioRecorder->isRecording();
		}
		virtual const char* GetRecordingAudioData() const
		{
			return reinterpret_cast<char*>( m_pAudioRecorder->getRecordedAudioData() );
		}

		virtual AudioFormat GetAudioFormat() const
		{
			return IrrKlang2BatAudioFormat( m_pAudioRecorder->getAudioFormat() );
		}
	private:
		std::function<void( const char* pAudioData, size_t length )> m_CustomHandledAudioCB;
		irrklang::IAudioRecorder* m_pAudioRecorder = nullptr;
	};

	std::vector<AudioDevice> Audio::GetSoundDeviceList()
	{
		std::vector<AudioDevice> devices;
		irrklang::ISoundDeviceList* pDeviceList = irrklang::createSoundDeviceList();

		int count = pDeviceList->getDeviceCount();
		devices.reserve( count );
		for( int i = 0; i < count; i++ )
		{
			AudioDevice device;
			device.id = pDeviceList->getDeviceID( i );
			device.description = pDeviceList->getDeviceDescription( i );
			devices.emplace_back( device );
		}

		pDeviceList->drop();

		return devices;
	}

	ISoundEngine* Audio::CreateSoundPlaybackDevice()
	{
		return new IrrKlangSoundEngine();
	}

	ISoundEngine * Audio::CreateSoundPlaybackDevice( const AudioDevice& device )
	{
		return new IrrKlangSoundEngine( device );
	}

	std::vector<AudioDevice> Audio::GetRecordingDeviceList()
	{
		std::vector<AudioDevice> devices;
		irrklang::ISoundDeviceList* pDeviceList = irrklang::createAudioRecorderDeviceList();

		int count = pDeviceList->getDeviceCount();
		devices.reserve( count );
		for( int i = 0; i < count; i++ )
		{
			AudioDevice device;
			device.id = pDeviceList->getDeviceID( i );
			device.description = pDeviceList->getDeviceDescription( i );
			devices.emplace_back( device );
		}

		pDeviceList->drop();

		return devices;
	}

	IRecordingEngine* Audio::CreateRecordingDevice( const ISoundEngine* pEngine )
	{
		return new IrrKlangRecordingEngine( pEngine );
	}

	IRecordingEngine* Audio::CreateRecordingDevice( const ISoundEngine* pEngine, const AudioDevice& device )
	{
		return new IrrKlangRecordingEngine( pEngine, device );
	}
}
