// BPTDib.h: interface for the dibsection class.
//
//	Copyright (c) 2002, Brad P. Taylor, LLC
//
//	All rights reserved, unauthorized reproduction prohibited
//
//	-- FILE NOTES --
//
//	The blitter should be broken up into two support methods.
//
//	1. Deals with HBITMAP's doesn't know about types.
//	2. Deals with the CDIBSection and related types.
//
// -- LICENSE --
//
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BPTDIB_H__4EE3AB65_FB91_4E19_9877_81FB6B6BDBFD__INCLUDED_)
#define AFX_BPTDIB_H__4EE3AB65_FB91_4E19_9877_81FB6B6BDBFD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ----------------------------------------------------------------------------

namespace BPT {

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

	//
	//	BLITFX
	//

	struct BLITFX {

		typedef SIZE size_type;
		typedef RECT rect_type;
		typedef POINT point_type;

		enum {

			HFLIP				= 0x00000001
			,VFLIP				= 0x00000002
			,WIDTH_SPECIFIED	= 0x00000004
			,HEIGHT_SPECIFIED	= 0x00000008

		};

		DWORD dwFlags;

		int nWidth;
		int nHeight;

		BLITFX(
			const DWORD flags, const int w = 0, const int h = 0
		) : dwFlags( flags ), nWidth( w ), nHeight( h ) { /* Empty */ }

	};

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	// DIBSECTION wrapper (16, 24, or 32 top down dib's only!!!)
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

	//
	//	CDIBSection
	//

	class CDIBSection {

	public: // traits

		enum {

			/*
				THESE SHOULD BE POLICY TEMPLATE PARAMS BUT VC++ 6.0 
				CHOKED HARD ON THIS CLASS BEING A TEMPLATE CLASS

			*/

			ALIGNMENT			= 4
			,DEFAULT_BPP		= 16      
			,DEFAULT_R_MASK		= 0
			,DEFAULT_G_MASK		= 0
			,DEFAULT_B_MASK		= 0

		};

		typedef CDIBSection this_type;
		typedef this_type & reference;
		typedef this_type * pointer;
		typedef const this_type & const_reference;
		typedef const this_type * const_pointer;

	private: // data

		struct {

			BITMAPINFOHEADER bmiHeader;

			DWORD ChannelMask_R;
			DWORD ChannelMask_G;
			DWORD ChannelMask_B;
				
		} m_BitmapInfo;

		void * m_BitmapBits;
		HBITMAP m_hBitmap;
		SIZE m_Size;
		int m_Pitch;

	private: // internal methods

		int CalcPitch( const int width, const int bpp ) const {

			int rawPitch = width * ((bpp+7)/8);

			int leftOver = (rawPitch % ALIGNMENT);

			if ( !leftOver ) {

				return rawPitch;

			}

			return rawPitch + (ALIGNMENT - leftOver);

		}

	public: // interface

		// --------------------------------------------------------------------

		CDIBSection() : m_hBitmap(0), m_Pitch(0) {

			::ZeroMemory( &m_BitmapInfo, sizeof(m_BitmapInfo) );

			m_Size.cx = 0;
			m_Size.cy = 0;
		
		}

		~CDIBSection() {

			Destroy();

		}

		// --------------------------------------------------------------------

		void Destroy() {

			if ( m_hBitmap ) {

				// Ask GDI to destroy it's concept of this bitmap

				::DeleteObject( (HGDIOBJ) m_hBitmap );

				// Release all of our information

				::ZeroMemory( &m_BitmapInfo, sizeof(m_BitmapInfo) );

				m_BitmapBits = 0;
				m_hBitmap = 0;

			}

			m_Size.cx = 0;
			m_Size.cy = 0;
			m_Pitch = 0;

		}

		// --------------------------------------------------------------------

		bool Create(
			const int width
			,const int height
			,HDC hdc = static_cast<HDC>(0)
			,const int bpp = DEFAULT_BPP
			,const DWORD r_channel_mask = DEFAULT_R_MASK
			,const DWORD g_channel_mask = DEFAULT_G_MASK
			,const DWORD b_channel_mask = DEFAULT_B_MASK
			,const bool bTopDown = true
		) {

			// Release any previous information
			// ----------------------------------------------------------------

			Destroy();

			// Fill in the bitmap info structure 
			// ----------------------------------------------------------------

			::ZeroMemory( &m_BitmapInfo, sizeof(m_BitmapInfo) );

			int pitch = CalcPitch( width, bpp );

			m_BitmapInfo.bmiHeader.biSize = sizeof(m_BitmapInfo.bmiHeader);
			m_BitmapInfo.bmiHeader.biBitCount = bpp;
			m_BitmapInfo.bmiHeader.biClrUsed = 0;
			m_BitmapInfo.bmiHeader.biClrImportant = 0;
			m_BitmapInfo.bmiHeader.biPlanes = 1;
			m_BitmapInfo.bmiHeader.biXPelsPerMeter = 0;
			m_BitmapInfo.bmiHeader.biYPelsPerMeter = 0; 
			m_BitmapInfo.bmiHeader.biWidth = width;
			m_BitmapInfo.bmiHeader.biHeight = (bTopDown) ? (-height) : (height);

			if ( (0 == ((bpp/8) & 1)) && (r_channel_mask && g_channel_mask && b_channel_mask) ) {

				m_BitmapInfo.bmiHeader.biCompression = BI_BITFIELDS;
				m_BitmapInfo.bmiHeader.biSizeImage = (pitch * height);

			} else {

				m_BitmapInfo.bmiHeader.biCompression = BI_RGB;
				m_BitmapInfo.bmiHeader.biSizeImage = 0;

			}

			m_BitmapInfo.ChannelMask_R = r_channel_mask;
			m_BitmapInfo.ChannelMask_G = g_channel_mask;
			m_BitmapInfo.ChannelMask_B = b_channel_mask;

			// Figure out what hdc to use
			// ----------------------------------------------------------------

			HDC usehdc = hdc;

			if ( !hdc ) {

				usehdc = ::GetDC( NULL );

			}

			// Ask GDI to create a DIBSECTION
			// ----------------------------------------------------------------

			m_hBitmap = ::CreateDIBSection( 
				usehdc,
				GetBITMAPINFO(),
				DIB_RGB_COLORS, 
				static_cast< void ** >( &m_BitmapBits ),
				0, 0
			);

			// If we created the hdc then destroy it.
			// ----------------------------------------------------------------

			if ( !hdc ) {

				::ReleaseDC( NULL, usehdc );

			}

			// Failure?
			// ----------------------------------------------------------------

			if ( !m_hBitmap ) {

				return false;

			}

			m_Pitch = pitch;
			m_Size.cx = width;
			m_Size.cy = height;
			
			return true;

		}

		// Query
		// --------------------------------------------------------------------

		bool TopDown() const {

			return (0 > m_BitmapInfo.bmiHeader.biHeight);

		}

		int BitDepth() const {

			return m_BitmapInfo.bmiHeader.biBitCount;

		}

		int Pitch() const {

			return m_Pitch;

		}

		RECT Rect() const {

			RECT rect = { 0, 0, m_Size.cx, m_Size.cy };

			return rect;

		}

		SIZE Size() const {

			return m_Size;

		}

		int Width() const {

			return m_Size.cx;

		}

		int Height() const {

			return m_Size.cy;

		}

		// --------------------------------------------------------------------

		DWORD GetRMask() const {

			return m_BitmapInfo.ChannelMask_R;

		}

		DWORD GetGMask() const {

			return m_BitmapInfo.ChannelMask_G;

		}

		DWORD GetBMask() const {

			return m_BitmapInfo.ChannelMask_B;

		}

		// --------------------------------------------------------------------

		HBITMAP GetHBitmap() const {

			return m_hBitmap;

		}

		const BITMAPINFO * GetBITMAPINFO() const {

			return reinterpret_cast<const BITMAPINFO *>( &m_BitmapInfo );

		}

		void * GetBitmapBits() const {

			return m_BitmapBits;

		}

		// --------------------------------------------------------------------

		template< class T > CreateCompatible(
			T & bitmap
			,const int width
			,const int height
			,HDC hdc = static_cast<HDC>(0)
			,const bool bTopDown = true
		) {

			return Create(
				width
				,height
				,hdc
				,TPixelTraits<T::pixel_type>::bpp
				,TPixelTraits<T::pixel_type>::r_isolation_mask
				,TPixelTraits<T::pixel_type>::g_isolation_mask
				,TPixelTraits<T::pixel_type>::b_isolation_mask
				,bTopDown
			);

		}

		template<class T> bool Map( T & bitmap, const bool bStrict = false ) const {

			// general error checking
			// ----------------------------------------------------------------

			if ( !m_BitmapBits ) {

				return false;

			}

			// validate pixel storage size before mapping
			// ----------------------------------------------------------------

			if ( TPixelTraits<T::pixel_type>::bpp != BitDepth() ) {

				return false;

			}

			if ( bStrict ) {

				if ( 

					(TPixelTraits<T::pixel_type>::r_isolation_mask != m_BitmapInfo.ChannelMask_R)
					|| (TPixelTraits<T::pixel_type>::g_isolation_mask != m_BitmapInfo.ChannelMask_G)
					|| (TPixelTraits<T::pixel_type>::b_isolation_mask != m_BitmapInfo.ChannelMask_B)
					) {

					return false;

				}

			}

			// Finally map either a top-down or bottom-up.
			// ----------------------------------------------------------------

			if ( 0 > m_BitmapInfo.bmiHeader.biHeight ) {

				return bitmap.Map(
					reinterpret_cast<BYTE *>(m_BitmapBits), m_Size, m_Pitch, false, false
				);

			}

			// must be bottom up

			return bitmap.Map(
				reinterpret_cast<BYTE *>(m_BitmapBits) + ((m_Size.cy - 1) * m_Pitch)
				,m_Size
				,-m_Pitch
				,false
				,false
			);

		}

	}; /* CDIBSection<> */

	// ========================================================================

	//
	//	T_BlitAt()
	//

	template< class DIBSECTION > void
	T_BlitAt(
		HDC hdc, const int x, const int y, DIBSECTION & dibSection
		,const BLITFX * pBlitFx = 0, const RECT * pOptionalSrcSubRect = 0
	) {

		// Determine the operation size

		RECT srcLimitRect = dibSection.Rect();

		RECT operationRect;

		if ( pOptionalSrcSubRect ) {

			if ( !IntersectRect( &operationRect, pOptionalSrcSubRect, &srcLimitRect ) ) {

				return /* NOP */;

			}

		} else {

			operationRect = srcLimitRect;

		}

		// Get down to business

		HBITMAP hBitmap = dibSection.GetHBitmap();

		if ( hBitmap ) {

			int w = operationRect.right - operationRect.left;
			int h = operationRect.bottom - operationRect.top;

			HDC memDC = ::CreateCompatibleDC( hdc );

			if ( memDC ) {

				HBITMAP hbmOld = (HBITMAP)::SelectObject( memDC, hBitmap );

				if ( pBlitFx) {

					int draw_cx = (BLITFX::WIDTH_SPECIFIED & pBlitFx->dwFlags) ? pBlitFx->nWidth : w;
					int draw_cy = (BLITFX::HEIGHT_SPECIFIED & pBlitFx->dwFlags) ? pBlitFx->nHeight : h;
					int draw_cx_multiplier = (BLITFX::HFLIP & pBlitFx->dwFlags) ? -1 : +1;
					int draw_cy_multiplier = (BLITFX::VFLIP & pBlitFx->dwFlags) ? -1 : +1;

					::StretchBlt(
						hdc, x, y, 
						(draw_cx * draw_cx_multiplier),
						(draw_cy * draw_cy_multiplier),
						memDC,
						operationRect.left,
						operationRect.top,
						w,
						h,
						SRCCOPY
					);

				} else {

					::BitBlt(
						hdc,
						x, y,
						w,
						h,
						memDC,
						operationRect.left,
						operationRect.top,
						SRCCOPY
					);

				}

				::SelectObject( memDC, hbmOld );
				::DeleteDC( memDC );

			}

		}

	}

	// ========================================================================

}; /* namespace BPT */

#endif // !defined(AFX_BPTDIB_H__4EE3AB65_FB91_4E19_9877_81FB6B6BDBFD__INCLUDED_)
