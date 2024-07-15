#pragma once

#include "askygg/core/Buffer.h"
#include "askygg/core/Memory.h"
#include "askygg/renderer/TextureUtils.h"

namespace askygg
{
	struct TextureCubeSpecification
	{
		ImageUtils::WrapMode			SamplerWrapS = ImageUtils::WrapMode::ClampToEdge;
		ImageUtils::WrapMode			SamplerWrapT = ImageUtils::WrapMode::ClampToEdge;
		ImageUtils::WrapMode			SamplerWrapR = ImageUtils::WrapMode::ClampToEdge;
		ImageUtils::FilterMode			MinFilter = ImageUtils::FilterMode::LinearMipLinear;
		ImageUtils::FilterMode			MagFilter = ImageUtils::FilterMode::Linear;
		ImageUtils::ImageInternalFormat InternalFormat = ImageUtils::ImageInternalFormat::RGBA32F;
		ImageUtils::ImageDataLayout		DataLayout = ImageUtils::ImageDataLayout::RGBA;
		ImageUtils::ImageDataType		DataType = ImageUtils::ImageDataType::Float;
		uint32_t						Dimension = 512;
		std::string						Name = "TextureCube";
	};

	struct Texture2DSpecification
	{
		ImageUtils::WrapMode			WrapModeS;
		ImageUtils::WrapMode			WrapModeT;
		ImageUtils::FilterMode			MinFilterMode;
		ImageUtils::FilterMode			MagFilterMode;
		ImageUtils::ImageInternalFormat InternalFormat;
		ImageUtils::ImageDataLayout		PixelLayoutFormat;
		ImageUtils::ImageDataType		DataType;
		uint32_t						Width, Height;
		std::string						Name = "Texture2D";
	};

	struct Texture2DArraySpecification
	{
		ImageUtils::WrapMode			WrapModeS;
		ImageUtils::WrapMode			WrapModeT;
		ImageUtils::FilterMode			MinFilterMode;
		ImageUtils::FilterMode			MagFilterMode;
		ImageUtils::ImageInternalFormat InternalFormat;
		ImageUtils::ImageDataLayout		PixelLayoutFormat;
		ImageUtils::ImageDataType		DataType;
		uint32_t						Width, Height, LayerCount;
		std::string						Name = "Texture2DArray";
	};

	struct Texture3DSpecification
	{
		ImageUtils::WrapMode			WrapModeS;
		ImageUtils::WrapMode			WrapModeT;
		ImageUtils::WrapMode			WrapModeR;
		ImageUtils::FilterMode			MinFilterMode;
		ImageUtils::FilterMode			MagFilterMode;
		ImageUtils::ImageInternalFormat InternalFormat;
		ImageUtils::ImageDataLayout		PixelLayoutFormat;
		ImageUtils::ImageDataType		DataType;
		uint32_t						Width, Height, Depth;
		std::string						Name = "Texture3D";
	};

	class Texture2D
	{
	public:
		Texture2D(const Texture2DSpecification& specification);
		Texture2D(const Texture2DSpecification& specification, void* data);
		Texture2D(const std::string& filePath, const Texture2DSpecification& specification);
		~Texture2D();

		void Invalidate();
		void ClearImage() const;
		void Resize(uint32_t width, uint32_t height);

		uint32_t					  GetID() const { return m_ID; }
		const Texture2DSpecification& GetSpecification() const { return m_Specification; }

		std::pair<uint32_t, uint32_t> GetMipSize(uint32_t mip) const;
		uint32_t					  GetMipLevelCount() const;
		uint32_t					  GetWidth() const { return m_Specification.Width; }
		uint32_t					  GetHeight() const { return m_Specification.Height; }
		std::string					  GetName() const { return m_Name; }
		std::string					  GetFilePath() const { return m_FilePath; }
		Buffer						  GetData() const { return m_ImageData; }
		void						  Save(const std::string& filePath, bool flipVertically = false) const;
		static void					  SaveToFile(uint32_t TextureID, const std::string& FilePath);

		static void SaveFramebufferAttachment(const std::string& filePath, uint32_t id,
			uint32_t bytesPerPixel, uint32_t width, uint32_t height);
		static void ReadColorData(const Buffer& buffer, int32_t width, int32_t height, uint32_t id);

		static void BindTextureIDToSamplerSlot(uint32_t slot, uint32_t id);
		static void BindTextureIDToImageSlot(uint32_t id, uint32_t unit, uint32_t level,
			ImageUtils::TextureAccessLevel		access,
			ImageUtils::TextureShaderDataFormat shaderDataFormat);

		void BindToSamplerSlot(uint32_t slot) const;
		void BindToImageSlot(uint32_t unit, uint32_t level, ImageUtils::TextureAccessLevel access,
			ImageUtils::TextureShaderDataFormat shaderDataFormat);

		static void ClearBinding();

		void SetData(void* data, uint32_t size) const;

	private:
		Buffer				   m_ImageData;
		Texture2DSpecification m_Specification;
		uint32_t			   m_ID{ UINT32_MAX };
		std::string			   m_FilePath;
		std::string			   m_Name;
	};

	class Texture2DArray
	{
	public:
		Texture2DArray(const Texture2DArraySpecification& Specification);
		~Texture2DArray();

		void Invalidate();
		void ResizeLayers(uint32_t width, uint32_t height);

		uint32_t						   GetID() const { return m_ID; }
		const Texture2DArraySpecification& GetSpecification() const { return m_Specification; }

		std::pair<uint32_t, uint32_t> GetMipSize(uint32_t mip) const;
		uint32_t					  GetMipLevelCount() const;
		uint32_t					  GetWidth() const { return m_Specification.Width; }
		uint32_t					  GetHeight() const { return m_Specification.Height; }
		std::string					  GetName() const { return m_Name; }
		void						  SetDataAtLayer(void* data, size_t size, uint32_t layer);
		void						  SetDataAllLayers(void* data, size_t size);

		static void BindTextureIDToSamplerSlot(uint32_t RendererID, uint32_t Slot);
		static void ClearBinding();
		void		BindLayerToImageSlot(uint32_t unit, uint32_t level, uint32_t layer,
				   ImageUtils::TextureAccessLevel	   access,
				   ImageUtils::TextureShaderDataFormat shaderDataFormat);
		void		BindAllLayersToImageSlot(uint32_t unit, uint32_t level,
				   ImageUtils::TextureAccessLevel	   access,
				   ImageUtils::TextureShaderDataFormat shaderDataFormat);

	private:
		Buffer						m_ImageData;
		Texture2DArraySpecification m_Specification;
		uint32_t					m_ID{};
		std::string					m_FilePath;
		std::string					m_Name;
	};

	class Texture2DImageView
	{
	public:
		Texture2DImageView(const Ref<Texture2D>& original, uint32_t baseMip, uint32_t mipCount,
			uint32_t baseLayer = 1, uint32_t layerCount = 0);
		~Texture2DImageView();

		void		ChangeToMip(const Ref<Texture2D>& original, uint32_t baseMip, uint32_t mipCount,
				   uint32_t baseLayer = 1, uint32_t layerCount = 0);
		void		Bind() const;
		static void Unbind();

		uint32_t GetBaseMip() const { return m_BaseMip; }
		uint32_t GetID() const { return m_ID; }

	private:
		uint32_t m_BaseMip = 0;
		uint32_t m_ID;
	};

	class Texture3D
	{
	public:
		Texture3D(const Texture3DSpecification& Specification);
		~Texture3D();

		void		BindToSamplerSlot(uint32_t slot = 0) const;
		static void Unbind();
		void		BindToImageSlot(uint32_t unit, uint32_t level, ImageUtils::TextureAccessLevel access,
				   ImageUtils::TextureShaderDataFormat shaderDataFormat) const;

		uint32_t	GetID() const { return m_ID; }
		uint32_t	GetWidth() const { return m_Specification.Width; }
		uint32_t	GetHeight() const { return m_Specification.Height; }
		uint32_t	GetDepth() const { return m_Specification.Depth; }
		std::string GetName() const { return m_Name; }
		std::string GetFilePath() const { return m_FilePath; }

		int WriteToFile(const std::string& assetPath) const;

		const Texture3DSpecification& GetSpecification() const { return m_Specification; }

		std::pair<uint32_t, uint32_t> GetMipSize(uint32_t mip) const;
		uint32_t					  GetMipLevelCount() const;

	private:
		Texture3DSpecification m_Specification;
		uint32_t			   m_ID;
		std::string			   m_FilePath;
		std::string			   m_Name;
	};

	class TextureCube
	{
	public:
		TextureCube(const TextureCubeSpecification& spec, const std::vector<std::string>& cubeFaceFiles);
		TextureCube(const TextureCubeSpecification& spec);
		~TextureCube();

		void Invalidate(const TextureCubeSpecification& Specification);

		void		BindToSamplerSlot(uint32_t slot = 0) const;
		static void Clear();
		void		BindToImageSlot(uint32_t Binding, uint32_t MipLevel,
				   ImageUtils::TextureAccessLevel	   AccessLevel,
				   ImageUtils::TextureShaderDataFormat ShaderDataFormat) const;

		std::pair<uint32_t, uint32_t> GetMipSize(uint32_t Mip) const;
		uint32_t					  GetMipLevelCount() const;

		uint32_t	GetID() const { return m_ID; }
		uint32_t	GetDimension() const { return m_Specification.Dimension; }
		std::string GetName() const { return m_Name; }

		void SetData(uint32_t* Data, size_t Size);

	private:
		TextureCubeSpecification m_Specification;
		uint32_t				 m_ID;
		std::string				 m_Name;
	};

	class TextureLibrary
	{
	public:
		static Texture2D&		Get2DFromID(uint32_t ID);
		static Ref<TextureCube> GetCubeFromID(uint32_t ID);
		static bool				Has2DFromID(uint32_t id);

		static Ref<Texture2D>		 LoadTexture2D(const std::string& filePath = "");
		static Ref<Texture2D>		 LoadTexture2D(const Texture2DSpecification& spec,
				   const std::string&											 filePath = "");
		static Ref<Texture2D>		 LoadTexture2D(const Texture2DSpecification& Spec, void* Data);
		static void					 AddTexture2D(const Ref<Texture2D>& texture);
		static const Ref<Texture2D>& Get2D(const std::string& name);
		static const Ref<Texture2D>& TryGet2D(const std::string& name,
			bool												 FallBackIsWhiteTexture = true);
		static void					 BindTexture2DToSlot(const std::string& TwoDimensionTextureName, uint32_t Slot);
		static bool					 Has2D(const std::string& Name);

		static Ref<TextureCube>		   LoadTextureCube(const TextureCubeSpecification& Spec,
				   bool																   InvalidateIfExists = false);
		static void					   InvalidateCube(const TextureCubeSpecification& spec);
		static void					   AddTextureCube(const Ref<TextureCube>& texture);
		static const Ref<TextureCube>& GetCube(const std::string& name);
		static const Ref<TextureCube>& TryGetCube(const std::string& name);
		static void					   BindTextureCubeToSlot(const std::string& CubeTextureName, uint32_t Slot);
		static bool					   HasCube(const std::string& Name);
		static uint32_t				   TryGetCubeIDFromName(const std::string& Name);
		static uint32_t				   TryGet2DIDFromName(const std::string& Name, bool FallBackIsWhiteTexture = true);
		static uint32_t				   TryGet2DArrayIDFromName(const std::string& Name);

		static const Ref<Texture2DArray>& Get2DArray(const std::string& name);
		static const Ref<Texture2DArray>& TryGet2DArray(const std::string& name);
		static bool						  Has2DArray(const std::string& Name);
		static Ref<Texture2DArray>		  LoadTexture2DArray(const Texture2DArraySpecification& Spec);
		static void						  AddTexture2DArray(const Ref<Texture2DArray>& texture);

		static std::unordered_map<std::string, Ref<Texture2D>> Get2DLibrary()
		{
			return s_NameToTexture2DLibrary;
		}
		static std::unordered_map<std::string, Ref<TextureCube>> GetCubeLibrary()
		{
			return s_NameToTextureCubeLibrary;
		}
		static std::unordered_map<std::string, Ref<Texture2DArray>> Get2DArrayLibrary()
		{
			return s_NameToTexture2DArrayLibrary;
		}

		static void		   BindTextureToSlot(uint32_t TexID, uint32_t Slot);
		static std::string GetNameFromID(uint32_t TextureID);
		static uint32_t	   GetIDFromName(const std::string& Name);

		static void LoadWhiteTexture();
		static void LoadBlackTexture();
		static void LoadBlackTextureCube();
		static void LoadWhiteTextureArray();

	private:
		static std::unordered_map<std::string, Ref<Texture2D>>		s_NameToTexture2DLibrary;
		static std::unordered_map<std::string, Ref<TextureCube>>	s_NameToTextureCubeLibrary;
		static std::unordered_map<std::string, Ref<Texture2DArray>> s_NameToTexture2DArrayLibrary;
		static std::unordered_map<uint32_t, std::string>			s_IdToNameLibrary;
		static std::unordered_map<std::string, uint32_t>			s_NameToIDLibrary;
	};
} // namespace askygg
