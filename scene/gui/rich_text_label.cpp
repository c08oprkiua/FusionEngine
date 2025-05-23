/*************************************************************************/
/*  rich_text_label.cpp                                                  */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#include "rich_text_label.h"
#include "scene/scene_string_names.h"
#include "os/keyboard.h"
#include "os/os.h"
RichTextLabel::Item *RichTextLabel::_get_next_item(Item* p_item) {

	if (p_item->subitems.size()) {

		return p_item->subitems.front()->get();
	} else if (!p_item->parent) {
		return NULL;
	} else if (p_item->E->next()) {

		return p_item->E->next()->get();
	} else {
		//go up until something with a next is found
		while (p_item->parent && !p_item->E->next()) {
			p_item=p_item->parent;
		}


		if (p_item && p_item->parent)
			return p_item->E->next()->get();
		else
			return NULL;

	}

	return NULL;

}

void RichTextLabel::_process_line(int &y, int p_width, int p_line, ProcessMode p_mode,const Ref<Font> &p_base_font,const Color &p_base_color,const Point2i& p_click_pos,Item **r_click_item,int *r_click_char,bool *r_outside) {

	RID ci;
	if (r_outside)
		*r_outside=false;
	if (p_mode==PROCESS_DRAW) {
		ci=get_canvas_item();

		if (r_click_item)
			*r_click_item=NULL;

	}
	Line &l = lines[p_line];
	Item *it = l.from;


	int line_ofs=0;
	int margin=_find_margin(it,p_base_font);
	Align align=_find_align(it);;
	int line=0;

	if (p_mode!=PROCESS_CACHE) {

		ERR_FAIL_INDEX(line,l.offset_caches.size());
		line_ofs = l.offset_caches[line];
	}

	int wofs=margin;

	if (p_mode!=PROCESS_CACHE && align!=ALIGN_FILL)
		wofs+=line_ofs;

	int begin=wofs;

	Ref<Font> cfont = _find_font(it);
	if (cfont.is_null())
		cfont=p_base_font;

	//line height should be the font height for the first time, this ensures that an empty line will never have zero height and succesive newlines are displayed
	int line_height=cfont->get_height();

	Variant meta;


#define NEW_LINE \
{\
	if (p_mode!=PROCESS_CACHE) {\
		line++;\
		if (line < l.offset_caches.size())\
			line_ofs=l.offset_caches[line];\
		wofs=margin;\
		if (align!=ALIGN_FILL)\
			wofs+=line_ofs;\
	} else {\
		int used=wofs-margin;\
		switch(align) {\
			case ALIGN_LEFT: l.offset_caches.push_back(0); break;\
			case ALIGN_CENTER: l.offset_caches.push_back(((p_width-margin)-used)/2); break;\
			case ALIGN_RIGHT: l.offset_caches.push_back(((p_width-margin)-used)); break;\
			case ALIGN_FILL: l.offset_caches.push_back(p_width-wofs); break;\
		}\
		l.height_caches.push_back(line_height);\
	}\
	y+=line_height+get_constant(SceneStringNames::line_separation);\
	line_height=0;\
	wofs=begin;\
	if (p_mode!=PROCESS_CACHE) {\
		lh=line<l.height_caches.size()?l.height_caches[line]:1;\
	}\
	if (p_mode==PROCESS_POINTER && r_click_item && p_click_pos.y>=y && p_click_pos.y<=y+lh && p_click_pos.x<wofs) {\
		if (r_outside) *r_outside=true;\
		*r_click_item=it;\
		*r_click_char=rchar;\
		return;\
	}\
}


#define ENSURE_WIDTH(m_width) \
	if (wofs + m_width > p_width) {\
		if (p_mode==PROCESS_POINTER && r_click_item && p_click_pos.y>=y && p_click_pos.y<=y+lh && p_click_pos.x>wofs) {\
			if (r_outside) *r_outside=true;	\
			*r_click_item=it;\
			*r_click_char=rchar;\
			return;\
		}\
		NEW_LINE\
	}


#define ADVANCE(m_width) \
{\
	if (p_mode==PROCESS_POINTER && r_click_item && p_click_pos.y>=y && p_click_pos.y<=y+lh && p_click_pos.x>=wofs && p_click_pos.x<wofs+m_width) {\
		if (r_outside) *r_outside=false;	\
		*r_click_item=it;\
		*r_click_char=rchar;\
		return;\
	}\
	wofs+=m_width;\
}

#define CHECK_HEIGHT( m_height ) \
if (m_height > line_height) {\
	line_height=m_height;\
}

	Color selection_fg;
	Color selection_bg;

	if (p_mode==PROCESS_DRAW) {


		selection_fg = get_color("font_color_selected");
		selection_bg = get_color("selection_color");
	}
	int rchar=0;
	int lh=0;

	while (it) {

		switch(it->type) {

			case ITEM_TEXT: {

				ItemText *text = static_cast<ItemText*>(it);

				Ref<Font> font=_find_font(it);
				if (font.is_null())
					font=p_base_font;

				const CharType *c = text->text.c_str();				
				const CharType *cf=c;
				int fh=font->get_height();
				int ascent = font->get_ascent();
				Color color;
				bool underline=false;

				if (p_mode==PROCESS_DRAW) {
					color=_find_color(text,p_base_color);
					underline=_find_underline(text);
					if (_find_meta(text,&meta)) {

						underline=true;
					}

				}

				rchar=0;
				while(*c) {

					int end=0;
					int w=0;

					lh=0;
					if (p_mode!=PROCESS_CACHE) {
						lh=line<l.height_caches.size()?l.height_caches[line]:1;
					}

					while (c[end]!=0 && !(end && c[end-1]==' ' && c[end]!=' ')) {
						int cw = font->get_char_size(c[end],c[end+1]).width;
						w+=cw;
						end++;
					}

					ENSURE_WIDTH(w);					



					{


						int ofs=0;
						for(int i=0;i<end;i++) {
							int pofs=wofs+ofs;




							if (p_mode==PROCESS_POINTER && r_click_char && p_click_pos.y>=y && p_click_pos.y<=y+lh) {
								//int o = (wofs+w)-p_click_pos.x;

								int cw=font->get_char_size(c[i],c[i+1]).x;

								if (p_click_pos.x-cw/2>pofs) {

									rchar=int((&c[i])-cf);
									//print_line("GOT: "+itos(rchar));


									//if (i==end-1 && p_click_pos.x+cw/2 > pofs)
									//	rchar++;
									//int o = (wofs+w)-p_click_pos.x;

								//	if (o>cw/2)
								//		rchar++;
								}


								ofs+=cw;
							} else if (p_mode==PROCESS_DRAW) {

								bool selected=false;
								if (selection.active) {

									int cofs = (&c[i])-cf;
									if ((text->index > selection.from->index || (text->index == selection.from->index && cofs >=selection.from_char)) && (text->index < selection.to->index || (text->index == selection.to->index && cofs <=selection.to_char))) {
										selected=true;
									}
								}

								int cw;

								if (selected) {

									cw = font->get_char_size(c[i],c[i+1]).x;
									draw_rect(Rect2(pofs,y,cw,lh),selection_bg);
									font->draw_char(ci,Point2(pofs,y+lh-(fh-ascent)),c[i],c[i+1],selection_fg);

								} else {
									cw=font->draw_char(ci,Point2(pofs,y+lh-(fh-ascent)),c[i],c[i+1],color);
								}

								if (underline) {
									Color uc=color;
									uc.a*=0.3;
									VS::get_singleton()->canvas_item_add_line(ci,Point2(pofs,y+ascent+2),Point2(pofs+cw,y+ascent+2),uc);
								}
								ofs+=cw;
							}

						}
					}


					ADVANCE(w);
					CHECK_HEIGHT(fh); //must be done somewhere
					c=&c[end];
				}


			} break;
			case ITEM_IMAGE: {

				lh=0;
				if (p_mode!=PROCESS_CACHE)
					lh = line<l.height_caches.size()?l.height_caches[line]:1;

				ItemImage *img = static_cast<ItemImage*>(it);

				Ref<Font> font=_find_font(it);
				if (font.is_null())
					font=p_base_font;

				if (p_mode==PROCESS_POINTER && r_click_char)
					*r_click_char=0;

				ENSURE_WIDTH( img->image->get_width() );

				if (p_mode==PROCESS_DRAW) {
					img->image->draw(ci,Point2(wofs,y+lh-font->get_descent()-img->image->get_height()));
				}

				ADVANCE( img->image->get_width() );
				CHECK_HEIGHT( (img->image->get_height()+font->get_descent()) );

			} break;
			case ITEM_NEWLINE: {


				lh=0;
				if (p_mode!=PROCESS_CACHE)
					lh = line<l.height_caches.size()?l.height_caches[line]:1;


#if 0
				if (p_mode==PROCESS_POINTER && r_click_item ) {
					//previous last "wrapped" line
					int pl = line-1;
					if (pl<0 || lines[pl].height_caches.size()==0)
						break;
					int py=lines[pl].offset_caches[ lines[pl].offset_caches.size() -1 ];
					int ph=lines[pl].height_caches[ lines[pl].height_caches.size() -1 ];
					print_line("py: "+itos(py));
					print_line("ph: "+itos(ph));

					rchar=0;
					if (p_click_pos.y>=py && p_click_pos.y<=py+ph) {
						if (r_outside) *r_outside=true;
						*r_click_item=it;
						*r_click_char=rchar;
						return;
					}
				}

#endif
			} break;

			default: {}

		}


		Item *itp = it;

		it = _get_next_item(it);

		if (p_mode == PROCESS_POINTER && r_click_item && itp && !it && p_click_pos.y>y+lh) {
			//at the end of all, return this
			if (r_outside) *r_outside=true;
			*r_click_item=itp;
			*r_click_char=rchar;
			return;
		}

		if (it && (p_line+1 < lines.size()) && lines[p_line+1].from==it) {

			if (p_mode==PROCESS_POINTER && r_click_item && p_click_pos.y>=y && p_click_pos.y<=y+lh) {
				//went to next line, but pointer was on the previous one
				if (r_outside) *r_outside=true;
				*r_click_item=itp;
				*r_click_char=rchar;
				return;
			}

			break;
		}
	}

	NEW_LINE;

#undef NEW_LINE
#undef ENSURE_WIDTH
#undef ADVANCE
#undef CHECK_HEIGHT

}

void RichTextLabel::_scroll_changed(double) {

	if (updating_scroll)
		return;

	if (scroll_follow && vscroll->get_val()>=(vscroll->get_max()-vscroll->get_page()))
		scroll_following=true;
	else
		scroll_following=false;

	update();

}

void RichTextLabel::_update_scroll() {

	int total_height=0;
	if (lines.size())
		total_height=lines[lines.size()-1].height_accum_cache;

	bool exceeds = total_height > get_size().height && scroll_active;


	if (exceeds!=scroll_visible) {

		if (exceeds) {
			scroll_visible=true;
			first_invalid_line=0;
			scroll_w=vscroll->get_combined_minimum_size().width;
			vscroll->show();
			vscroll->set_anchor_and_margin( MARGIN_LEFT, ANCHOR_END,scroll_w);
			_validate_line_caches();

		} else {

			scroll_visible=false;
			vscroll->hide();
			scroll_w=0;
			_validate_line_caches();
		}

	}

}

void RichTextLabel::_notification(int p_what) {

	switch (p_what) {

		case NOTIFICATION_RESIZED: {

			first_invalid_line=0; //invalidate ALL
			update();

		} break;
		case NOTIFICATION_DRAW: {

			_validate_line_caches();
			_update_scroll();


			RID ci=get_canvas_item();
			Size2 size = get_size();

			VisualServer::get_singleton()->canvas_item_set_clip(ci,true);

			if (has_focus()) {
				VisualServer::get_singleton()->canvas_item_add_clip_ignore(ci,true);
				draw_style_box(get_stylebox("focus"),Rect2(Point2(),size));
				VisualServer::get_singleton()->canvas_item_add_clip_ignore(ci,false);
			}

			int ofs = vscroll->get_val();

			//todo, change to binary search

			int from_line = 0;
			while (from_line<lines.size()) {

				if (lines[from_line].height_accum_cache>=ofs)
					break;
				from_line++;
			}

			if (from_line>=lines.size())
				break; //nothing to draw

			int y = (lines[from_line].height_accum_cache - lines[from_line].height_cache) - ofs;
			Ref<Font> base_font=get_font("default_font");
			Color base_color=get_color("default_color");

			while (y<size.height && from_line<lines.size()) {

				_process_line(y,size.width-scroll_w,from_line,PROCESS_DRAW,base_font,base_color);
				from_line++;
			}
		}
	}
}


void RichTextLabel::_find_click(const Point2i& p_click,Item **r_click_item,int *r_click_char,bool *r_outside) {

	if (r_click_item)
		*r_click_item=NULL;

	Size2 size = get_size();

	int ofs = vscroll->get_val();

	//todo, change to binary search
	int from_line = 0;

	while (from_line<lines.size()) {

		if (lines[from_line].height_accum_cache>=ofs)
			break;
		from_line++;
	}


	if (from_line>=lines.size())
		return;


	int y = (lines[from_line].height_accum_cache - lines[from_line].height_cache) - ofs;
	Ref<Font> base_font=get_font("default_font");
	Color base_color=get_color("default_color");


	while (y<size.height && from_line<lines.size()) {

		_process_line(y,size.width-scroll_w,from_line,PROCESS_POINTER,base_font,base_color,p_click,r_click_item,r_click_char,r_outside);
		if (r_click_item && *r_click_item)
			return;
		from_line++;
	}


}


Control::CursorShape RichTextLabel::get_cursor_shape(const Point2& p_pos) const {

	if (!underline_meta || selection.click)
		return CURSOR_ARROW;

	if (first_invalid_line<lines.size())
		return CURSOR_ARROW; //invalid

	int line=0;
	Item *item=NULL;

	((RichTextLabel*)(this))->_find_click(p_pos,&item,&line);


	if (item && ((RichTextLabel*)(this))->_find_meta(item,NULL))
		return CURSOR_POINTING_HAND;

	return CURSOR_ARROW;
}


void RichTextLabel::_input_event(InputEvent p_event) {

	switch(p_event.type) {

		case InputEvent::MOUSE_BUTTON: {

			if (first_invalid_line<lines.size())
				return;

			const InputEventMouseButton& b = p_event.mouse_button;

			if (b.button_index==BUTTON_LEFT) {

				if (true) {


					if (b.pressed && !b.doubleclick) {
						int line=0;
						Item *item=NULL;

						bool outside;
						_find_click(Point2i(b.x,b.y),&item,&line,&outside);

						if (item) {

							Variant meta;
							if (!outside && _find_meta(item,&meta)) {
								//meta clicked

								emit_signal("meta_clicked",meta);
							} else if (selection.enabled) {

								selection.click=item;
								selection.click_char=line;

							}

						}

					} else if (!b.pressed) {

						selection.click=NULL;
					}
				}
			}

			if (b.button_index==BUTTON_WHEEL_UP) {

				if (scroll_active)
					vscroll->set_val( vscroll->get_val()-vscroll->get_page()/8 );
			}
			if (b.button_index==BUTTON_WHEEL_DOWN) {

				if (scroll_active)
					vscroll->set_val( vscroll->get_val()+vscroll->get_page()/8 );
			}
		} break;
		case InputEvent::KEY: {

			const InputEventKey &k=p_event.key;
			if (k.pressed) {
				bool handled=true;
				switch(k.scancode) {
					case KEY_PAGEUP: {

						if (vscroll->is_visible())
							vscroll->set_val( vscroll->get_val() - vscroll->get_page() );
					} break;
					case KEY_PAGEDOWN: {

						if (vscroll->is_visible())
							vscroll->set_val( vscroll->get_val() + vscroll->get_page() );
					} break;
					case KEY_UP: {

						if (vscroll->is_visible())
							vscroll->set_val( vscroll->get_val() - get_font("default_font")->get_height() );
					} break;
					case KEY_DOWN: {

						if (vscroll->is_visible())
							vscroll->set_val( vscroll->get_val() + get_font("default_font")->get_height() );
					} break;
					case KEY_HOME: {

						if (vscroll->is_visible())
							vscroll->set_val( 0 );
					} break;
					case KEY_END: {

						if (vscroll->is_visible())
							vscroll->set_val( vscroll->get_max() );
					} break;
					case KEY_INSERT:
					case KEY_C: {

						if (k.mod.command) {
							selection_copy();
						} else {
							handled=false;
						}

					} break;
					default: handled=false;
				}

				if (handled)
					accept_event();
			}

		} break;
		case InputEvent::MOUSE_MOTION: {

			if (first_invalid_line<lines.size())
				return;

			const InputEventMouseMotion& m = p_event.mouse_motion;

			if (selection.click) {

				int line=0;
				Item *item=NULL;
				_find_click(Point2i(m.x,m.y),&item,&line);
				if (!item)
					return; // do not update


				selection.from=selection.click;
				selection.from_char=selection.click_char;

				selection.to=item;
				selection.to_char=line;

				bool swap=false;
				if (selection.from->index > selection.to->index )
					swap=true;
				else if (selection.from->index == selection.to->index) {
					if (selection.from_char > selection.to_char)
						swap=true;
					else if (selection.from_char == selection.to_char) {

						selection.active=false;
						return;
					}
				}

				if (swap) {
					SWAP( selection.from, selection.to );
					SWAP( selection.from_char, selection.to_char );
				}

				selection.active=true;
				update();

			}

		} break;
	}

}

Ref<Font> RichTextLabel::_find_font(Item *p_item) {

	Item *fontitem=p_item;

	while(fontitem) {

		if (fontitem->type==ITEM_FONT) {

			ItemFont *fi = static_cast<ItemFont*>(fontitem);
			return fi->font;
		}

		fontitem=fontitem->parent;
	}

	return Ref<Font>();
}

int RichTextLabel::_find_margin(Item *p_item,const Ref<Font>& p_base_font) {

	Item *item=p_item;

	int margin=0;

	while(item) {

		if (item->type==ITEM_INDENT) {

			Ref<Font> font=_find_font(item);
			if (font.is_null())
				font=p_base_font;

			ItemIndent *indent = static_cast<ItemIndent*>(item);

			margin+=indent->level*tab_size*font->get_char_size(' ').width;

		} else if (item->type==ITEM_LIST) {

			Ref<Font> font=_find_font(item);
			if (font.is_null())
				font=p_base_font;

		}

		item=item->parent;
	}

	return margin;
}


RichTextLabel::Align RichTextLabel::_find_align(Item *p_item) {

	Item *item=p_item;

	while(item) {

		if (item->type==ITEM_ALIGN) {

			ItemAlign *align = static_cast<ItemAlign*>(item);
			return align->align;

		}

		item=item->parent;
	}

	return default_align;
}

Color RichTextLabel::_find_color(Item *p_item,const Color& p_default_color) {

	Item *item=p_item;

	while(item) {

		if (item->type==ITEM_COLOR) {

			ItemColor *color = static_cast<ItemColor*>(item);
			return color->color;

		}

		item=item->parent;
	}

	return p_default_color;
}

bool RichTextLabel::_find_underline(Item *p_item) {

	Item *item=p_item;

	while(item) {

		if (item->type==ITEM_UNDERLINE) {

			return true;

		}

		item=item->parent;
	}

	return false;
}

bool RichTextLabel::_find_meta(Item *p_item,Variant *r_meta) {

	Item *item=p_item;

	while(item) {

		if (item->type==ITEM_META) {

			ItemMeta *meta = static_cast<ItemMeta*>(item);
			if (r_meta)
				*r_meta=meta->meta;
			return true;

		}

		item=item->parent;
	}

	return false;

}

void RichTextLabel::_validate_line_caches() {

	if (first_invalid_line==lines.size())
		return;

	//validate invalid lines!s
	Size2 size = get_size();

	Ref<Font> base_font=get_font("default_font");

	for(int i=first_invalid_line;i<lines.size();i++) {

		int y=0;
		_process_line(y,size.width-scroll_w,i,PROCESS_CACHE,base_font,Color());

		lines[i].height_cache=y;
		lines[i].height_accum_cache=y;

		if (i>0)
			lines[i].height_accum_cache+=lines[i-1].height_accum_cache;


	}

	int total_height=0;
	if (lines.size())
		total_height=lines[lines.size()-1].height_accum_cache;

	first_invalid_line=lines.size();

	updating_scroll=true;
	vscroll->set_max(total_height);
	vscroll->set_page(size.height);
	if (scroll_follow && scroll_following)
		vscroll->set_val(total_height-size.height);

	updating_scroll=false;

}


void RichTextLabel::_invalidate_current_line() {

	if (lines.size()-1 <= first_invalid_line) {

		first_invalid_line=lines.size()-1;
		update();
	}
}

void RichTextLabel::add_text(const String& p_text) {

	int pos=0;

	while (pos<p_text.length()) {

		int end=p_text.find("\n",pos);
		String line;
		bool eol=false;
		if (end==-1) {

			end=p_text.length();
		} else {

			eol=true;
		}

		if (pos==0 && end==p_text.length())
			line=p_text;
		else
			line=p_text.substr(pos,end-pos);

		if (line.length()>0) {

			if (current->subitems.size() && current->subitems.back()->get()->type==ITEM_TEXT) {
				//append text condition!
				ItemText *ti = static_cast<ItemText*>(current->subitems.back()->get());
				ti->text+=line;
				_invalidate_current_line();

			} else {
				//append item condition
				ItemText *item = memnew( ItemText );
				item->text=line;
				_add_item(item,false);

			}


		}

		if (eol) {

			ItemNewline *item = memnew( ItemNewline );
			item->line=lines.size();
			_add_item(item,false);
			lines.resize(lines.size()+1);
			lines[lines.size()-1].from=item;
			_invalidate_current_line();

		}

		pos=end+1;
	}
}

void RichTextLabel::_add_item(Item *p_item, bool p_enter) {

	p_item->parent=current;
	p_item->E=current->subitems.push_back(p_item);
	p_item->index=current_idx++;

	if (p_enter)
		current=p_item;

	if (lines[lines.size()-1].from==NULL) {
		lines[lines.size()-1].from=p_item;
	}

	_invalidate_current_line();

}

void RichTextLabel::add_image(const Ref<Texture>& p_image) {

	ERR_FAIL_COND(p_image.is_null());
	ItemImage *item = memnew( ItemImage );

	item->image=p_image;
	_add_item(item,false);

}

void RichTextLabel::add_newline() {

	ItemNewline *item = memnew( ItemNewline );
	item->line=lines.size();
	lines.resize(lines.size()+1);
	_add_item(item,false);

}

void RichTextLabel::push_font(const Ref<Font>& p_font) {

	ERR_FAIL_COND(p_font.is_null());
	ItemFont *item = memnew( ItemFont );

	item->font=p_font;
	_add_item(item,true);

}
void RichTextLabel::push_color(const Color& p_color) {

	ItemColor *item = memnew( ItemColor );

	item->color=p_color;
	_add_item(item,true);

}
void RichTextLabel::push_underline() {

	ItemUnderline *item = memnew( ItemUnderline );

	_add_item(item,true);

}

void RichTextLabel::push_align(Align p_align) {

	lines.resize(lines.size()+1);

	ItemAlign *item = memnew( ItemAlign );
	item->align=p_align;
	_add_item(item,true);

	ItemNewline *itemnl = memnew( ItemNewline );
	itemnl->line=lines.size()-1;
	_add_item(itemnl,false);

}

void RichTextLabel::push_indent(int p_level) {

	ERR_FAIL_COND(p_level<0);

	lines.resize(lines.size()+1);

	ItemIndent *item = memnew( ItemIndent );
	item->level=p_level;
	_add_item(item,true);

	ItemNewline *itemnl = memnew( ItemNewline );
	itemnl->line=lines.size()-1;
	_add_item(itemnl,false);

}

void RichTextLabel::push_list(ListType p_list) {

	ERR_FAIL_INDEX(p_list,3);

	ItemList *item = memnew( ItemList );

	item->list_type=p_list;
	_add_item(item,true);

}

void RichTextLabel::push_meta(const Variant& p_meta) {

	ItemMeta *item = memnew( ItemMeta );

	item->meta=p_meta;
	_add_item(item,true);

}

void RichTextLabel::pop() {

	ERR_FAIL_COND(!current->parent);
	current=current->parent;
}

void RichTextLabel::clear() {

	main->_clear_children();
	current=main;
	lines.clear();
	lines.resize(1);
	first_invalid_line=0;
	update();
	selection.click=NULL;
	selection.active=false;
	current_idx=1;

}

void RichTextLabel::set_tab_size(int p_spaces) {

	tab_size=p_spaces;
	first_invalid_line=0;
	update();
}

int RichTextLabel::get_tab_size() const {

	return tab_size;
}


void RichTextLabel::set_meta_underline(bool p_underline) {

	underline_meta=p_underline;
	update();
}

bool RichTextLabel::is_meta_underlined() const {

	return underline_meta;
}

void RichTextLabel::set_offset(int p_pixel) {

	vscroll->set_val(p_pixel);
}

void RichTextLabel::set_scroll_active(bool p_active) {

	if (scroll_active==p_active)
		return;

	scroll_active=p_active;
	update();
}

bool RichTextLabel::is_scroll_active() const {

	return scroll_active;
}

void RichTextLabel::set_scroll_follow(bool p_follow) {

	scroll_follow=p_follow;
	if (!vscroll->is_visible() || vscroll->get_val()>=(vscroll->get_max()-vscroll->get_page()))
		scroll_following=true;
}

bool RichTextLabel::is_scroll_following() const {

	return scroll_follow;
}

Error RichTextLabel::parse_bbcode(const String& p_bbcode) {


	clear();
	return append_bbcode(p_bbcode);
}

Error RichTextLabel::append_bbcode(const String& p_bbcode) {

	int pos = 0;

	List<String> tag_stack;
	Ref<Font> base_font=get_font("default_font");
	Color base_color=get_color("default_color");

	while(pos < p_bbcode.length()) {


		int brk_pos = p_bbcode.find("[",pos);

		if (brk_pos<0)
			brk_pos=p_bbcode.length();

		if (brk_pos > pos) {
			add_text(p_bbcode.substr(pos,brk_pos-pos));
		}

		if (brk_pos==p_bbcode.length())
			break; //nothing else o add

		int brk_end = p_bbcode.find("]",brk_pos+1);

		if (brk_end==-1) {
			//no close, add the rest
			add_text(p_bbcode.substr(brk_pos,p_bbcode.length()-brk_pos));
			break;
		}


		String tag = p_bbcode.substr(brk_pos+1,brk_end-brk_pos-1);


		if (tag.begins_with("/")) {

			bool tag_ok = tag_stack.size() && tag_stack.front()->get()==tag.substr(1,tag.length());





			if (!tag_ok) {

				add_text("[");
				pos++;
				continue;
			}

			tag_stack.pop_front();
			pos=brk_end+1;
			if (tag!="/img")
				pop();

		} else if (tag=="b") {

			//use bold font
			push_font(base_font);
			pos=brk_end+1;
			tag_stack.push_front(tag);
		} else if (tag=="i") {

			//use italics font
			push_font(base_font);
			pos=brk_end+1;
			tag_stack.push_front(tag);
		} else if (tag=="code") {

			//use monospace font
			push_font(base_font);
			pos=brk_end+1;
			tag_stack.push_front(tag);
		} else if (tag=="u") {

			//use underline
			push_underline();
			pos=brk_end+1;
			tag_stack.push_front(tag);
		} else if (tag=="s") {

			//use strikethrough (not supported underline instead)
			push_underline();
			pos=brk_end+1;
			tag_stack.push_front(tag);

		} else if (tag=="url") {

			//use strikethrough (not supported underline instead)
			int end=p_bbcode.find("[",brk_end);
			if (end==-1)
				end=p_bbcode.length();
			String url = p_bbcode.substr(brk_end+1,end-brk_end-1);
			push_meta(url);

			pos=brk_end+1;
			tag_stack.push_front(tag);
		} else if (tag.begins_with("url=")) {

			String url = tag.substr(4,tag.length());
			push_meta(url);
			pos=brk_end+1;
			tag_stack.push_front("url");
		} else if (tag=="img") {

			//use strikethrough (not supported underline instead)
			int end=p_bbcode.find("[",brk_end);
			if (end==-1)
				end=p_bbcode.length();
			String image = p_bbcode.substr(brk_end+1,end-brk_end-1);

			Ref<Texture> texture = ResourceLoader::load(image,"Texture");
			if (texture.is_valid())
				add_image(texture);

			pos=end;
			tag_stack.push_front(tag);
		} else if (tag.begins_with("color=")) {

			String col = tag.substr(6,tag.length());
			Color color;

			if (col.begins_with("#"))
				color=Color::html(col);
			else if (col=="aqua")
				color=Color::html("#00FFFF");
			else if (col=="black")
				color=Color::html("#000000");
			else if (col=="blue")
				color=Color::html("#0000FF");
			else if (col=="fuchsia")
				color=Color::html("#FF00FF");
			else if (col=="gray" || col=="grey")
				color=Color::html("#808080");
			else if (col=="green")
				color=Color::html("#008000");
			else if (col=="lime")
				color=Color::html("#00FF00");
			else if (col=="maroon")
				color=Color::html("#800000");
			else if (col=="navy")
				color=Color::html("#000080");
			else if (col=="olive")
				color=Color::html("#808000");
			else if (col=="purple")
				color=Color::html("#800080");
			else if (col=="red")
				color=Color::html("#FF0000");
			else if (col=="silver")
				color=Color::html("#C0C0C0");
			else if (col=="teal")
				color=Color::html("#008008");
			else if (col=="white")
				color=Color::html("#FFFFFF");
			else if (col=="yellow")
				color=Color::html("#FFFF00");
			else
				color=base_color;



			push_color(color);
			pos=brk_end+1;
			tag_stack.push_front("color");

		} else if (tag.begins_with("font=")) {

			String fnt = tag.substr(5,tag.length());


			Ref<Font> font = ResourceLoader::load(fnt,"Font");
			if (font.is_valid())
				push_font(font);
			else
				push_font(base_font);

			pos=brk_end+1;
			tag_stack.push_front("font");


		} else {

			add_text("["); //ignore
			pos=brk_pos+1;

		}
	}

	return OK;
}


void RichTextLabel::scroll_to_line(int p_line) {

	ERR_FAIL_INDEX(p_line,lines.size());
	_validate_line_caches();
	vscroll->set_val(lines[p_line].height_accum_cache);


}

int RichTextLabel::get_line_count() const {

	return lines.size();
}

void RichTextLabel::set_selection_enabled(bool p_enabled) {

	selection.enabled=p_enabled;
	if (!p_enabled) {
		if (selection.active) {
			selection.active=false;
			update();
		}
		set_focus_mode(FOCUS_NONE);
	} else {
		set_focus_mode(FOCUS_ALL);
	}

}

bool RichTextLabel::search(const String& p_string,bool p_from_selection) {

	ERR_FAIL_COND_V(!selection.enabled,false);
	Item *it=main;
	int charidx=0;

	if (p_from_selection && selection.active && selection.enabled) {
		it=selection.to;
		charidx=selection.to_char+1;
	}

	int line=-1;
	while(it) {

		if (it->type==ITEM_TEXT) {

			ItemText *t = static_cast<ItemText*>(it);
			int sp = t->text.find(p_string,charidx);
			if (sp!=-1) {
				selection.from=it;
				selection.from_char=sp;
				selection.to=it;
				selection.to_char=sp+p_string.length()-1;
				selection.active=true;
				update();

				if (line==-1) {

					while(it) {

						if (it->type==ITEM_NEWLINE) {

							line=static_cast<ItemNewline*>(it)->line;
							break;
						}

						it=_get_next_item(it);
					}

					if (!it)
						line=lines.size()-1;
				}

				scroll_to_line(line-2);

				return true;
			}
		} else if (it->type==ITEM_NEWLINE) {

			line=static_cast<ItemNewline*>(it)->line;
		}


		it=_get_next_item(it);
		charidx=0;

	}



	return false;

}

void RichTextLabel::selection_copy() {

	if (!selection.enabled)
		return;

	String text;

	RichTextLabel::Item *item=selection.from;

	while(item) {

		if (item->type==ITEM_TEXT) {

			String itext = static_cast<ItemText*>(item)->text;
			if (item==selection.from && item==selection.to) {
				text+=itext.substr(selection.from_char,selection.to_char-selection.from_char+1);
			} else if (item==selection.from) {
				text+=itext.substr(selection.from_char,itext.size());
			} else if (item==selection.to) {
				text+=itext.substr(0,selection.to_char+1);
			} else {
				text+=itext;
			}

		} else if (item->type==ITEM_NEWLINE) {
			text+="\n";
		}
		if (item==selection.to)
			break;

		item=_get_next_item(item);
	}

	if (text!="") {
		OS::get_singleton()->set_clipboard(text);
		print_line("COPY: "+text);
	}

}

bool RichTextLabel::is_selection_enabled() const {

	return selection.enabled;
}


void RichTextLabel::_bind_methods() {


	ObjectTypeDB::bind_method(_MD("_input_event"),&RichTextLabel::_input_event);
	ObjectTypeDB::bind_method(_MD("_scroll_changed"),&RichTextLabel::_scroll_changed);
	ObjectTypeDB::bind_method(_MD("add_text","text"),&RichTextLabel::add_text);
	ObjectTypeDB::bind_method(_MD("add_image","image:Texture"),&RichTextLabel::add_image);
	ObjectTypeDB::bind_method(_MD("newline"),&RichTextLabel::add_newline);
	ObjectTypeDB::bind_method(_MD("push_font","font"),&RichTextLabel::push_font);
	ObjectTypeDB::bind_method(_MD("push_color","color"),&RichTextLabel::push_color);
	ObjectTypeDB::bind_method(_MD("push_align","align"),&RichTextLabel::push_align);
	ObjectTypeDB::bind_method(_MD("push_indent","level"),&RichTextLabel::push_indent);
	ObjectTypeDB::bind_method(_MD("push_list","type"),&RichTextLabel::push_list);
	ObjectTypeDB::bind_method(_MD("push_meta","data"),&RichTextLabel::push_meta);
	ObjectTypeDB::bind_method(_MD("push_underline"),&RichTextLabel::push_underline);
	ObjectTypeDB::bind_method(_MD("pop"),&RichTextLabel::pop);

	ObjectTypeDB::bind_method(_MD("clear"),&RichTextLabel::clear);

	ObjectTypeDB::bind_method(_MD("set_meta_underline","enable"),&RichTextLabel::set_meta_underline);
	ObjectTypeDB::bind_method(_MD("is_meta_underlined"),&RichTextLabel::is_meta_underlined);

	ObjectTypeDB::bind_method(_MD("set_scroll_active","active"),&RichTextLabel::set_scroll_active);
	ObjectTypeDB::bind_method(_MD("is_scroll_active"),&RichTextLabel::is_scroll_active);

	ObjectTypeDB::bind_method(_MD("set_scroll_follow","follow"),&RichTextLabel::set_scroll_follow);
	ObjectTypeDB::bind_method(_MD("is_scroll_following"),&RichTextLabel::is_scroll_following);

	ObjectTypeDB::bind_method(_MD("set_tab_size","spaces"),&RichTextLabel::set_tab_size);
	ObjectTypeDB::bind_method(_MD("get_tab_size"),&RichTextLabel::get_tab_size);

	ObjectTypeDB::bind_method(_MD("set_selection_enabled","enabled"),&RichTextLabel::set_selection_enabled);
	ObjectTypeDB::bind_method(_MD("is_selection_enabled"),&RichTextLabel::is_selection_enabled);

	ObjectTypeDB::bind_method(_MD("parse_bbcode", "bbcode"),&RichTextLabel::parse_bbcode);
	ObjectTypeDB::bind_method(_MD("append_bbcode", "bbcode"),&RichTextLabel::append_bbcode);


	ADD_SIGNAL( MethodInfo("meta_clicked",PropertyInfo(Variant::NIL,"meta")));

	BIND_CONSTANT( ALIGN_LEFT );
	BIND_CONSTANT( ALIGN_CENTER );
	BIND_CONSTANT( ALIGN_RIGHT );
	BIND_CONSTANT( ALIGN_FILL );

	BIND_CONSTANT( LIST_NUMBERS );
	BIND_CONSTANT( LIST_LETTERS );
	BIND_CONSTANT( LIST_DOTS );

	BIND_CONSTANT( ITEM_MAIN );
	BIND_CONSTANT( ITEM_TEXT );
	BIND_CONSTANT( ITEM_IMAGE );
	BIND_CONSTANT( ITEM_NEWLINE );
	BIND_CONSTANT( ITEM_FONT );
	BIND_CONSTANT( ITEM_COLOR );
	BIND_CONSTANT( ITEM_UNDERLINE );
	BIND_CONSTANT( ITEM_ALIGN );
	BIND_CONSTANT( ITEM_INDENT );
	BIND_CONSTANT( ITEM_LIST );
	BIND_CONSTANT( ITEM_META );

}

RichTextLabel::RichTextLabel() {


	main = memnew( ItemMain );
	main->index=0;
	current=main;
	lines.resize(1);
	lines[0].from=main;
	first_invalid_line=0;
	tab_size=4;
	default_align=ALIGN_LEFT;
	underline_meta=true;

	scroll_visible=false;
	scroll_follow=false;
	scroll_following=false;
	updating_scroll=false;
	scroll_active=true;
	scroll_w=0;

	vscroll = memnew( VScrollBar );	
	add_child(vscroll);
	vscroll->set_drag_slave(String(".."));
	vscroll->set_step(1);
	vscroll->set_anchor_and_margin( MARGIN_TOP, ANCHOR_BEGIN, 0);
	vscroll->set_anchor_and_margin( MARGIN_BOTTOM, ANCHOR_END, 0);
	vscroll->set_anchor_and_margin( MARGIN_RIGHT, ANCHOR_END, 0);
	vscroll->connect("value_changed",this,"_scroll_changed");
	vscroll->set_step(1);
	vscroll->hide();
	current_idx=1;

	selection.click=NULL;
	selection.active=false;
	selection.enabled=false;

}

RichTextLabel::~RichTextLabel() {

	memdelete( main );
}
