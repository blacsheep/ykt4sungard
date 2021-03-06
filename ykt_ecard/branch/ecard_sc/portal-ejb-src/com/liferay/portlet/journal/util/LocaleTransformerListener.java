/**
 * Copyright (c) 2000-2005 Liferay, LLC. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package com.liferay.portlet.journal.util;

import com.liferay.util.LocaleUtil;
import com.liferay.util.StringPool;
import com.liferay.util.StringUtil;

import java.io.StringReader;

import java.util.List;
import java.util.Locale;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;

/**
 * <a href="LocaleTransformerListener.java.html"><b><i>View Source</i></b></a>
 *
 * @author  Raymond Auge
 * @version $Revision: 1.5 $
 *
 */
public class LocaleTransformerListener extends TransformerListener {

	public String onXml(String s) {
		_log.debug("onXml");

		s = localize(s);

		return s;
	}

	public String onXsl(String s) {
		_log.debug("onXsl");

		s = StringUtil.replace(s, "@language_id@", _requestedLocale);

		return s;
	}

	public String onOutput(String s) {
		_log.debug("onOutput");

		return s;
	}

	protected String localize(String xml) {
		_requestedLocale = getLanguageId();

		try {
			SAXReader reader = new SAXReader();

			Document doc = reader.read(new StringReader(xml));

			Element root = doc.getRootElement();

			String defaultLanguageId =
				LocaleUtil.toLanguageId(Locale.getDefault());

			String[] availableLocales = StringUtil.split(
				root.attributeValue("available-locales", defaultLanguageId));

			String defaultLocale = root.attributeValue(
				"default-locale", defaultLanguageId);

			boolean isSupportedLocale = false;

			for (int i = 0; i < availableLocales.length; i++) {
				if (availableLocales[i].equalsIgnoreCase(getLanguageId())) {
					isSupportedLocale = true;

					break;
				}
			}

			if (!isSupportedLocale) {
				setLanguageId(defaultLocale);
			}

			localize(root);

			xml = JournalUtil.formatXML(doc);
		}
		catch (Exception e) {
			_log.error(e);
		}

		return xml;
	}

	protected void localize(Element root) {
		List children = root.elements();

		int listIndex = children.size() - 1;

		while (listIndex >= 0) {
			Element child = (Element)children.get(listIndex);

			String languageId =
				child.attributeValue("language-id", getLanguageId());

			if (!languageId.equalsIgnoreCase(getLanguageId())) {
				root.remove(child);
			}
			else{
				localize(child);
			}

			listIndex--;
		}
	}

	private static final Log _log =
		LogFactory.getLog(LocaleTransformerListener.class);

	private String _requestedLocale = StringPool.BLANK;

}