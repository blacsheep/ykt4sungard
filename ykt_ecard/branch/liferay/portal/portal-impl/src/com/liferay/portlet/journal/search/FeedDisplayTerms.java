/**
 * Copyright (c) 2000-2008 Liferay, Inc. All rights reserved.
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

package com.liferay.portlet.journal.search;

import com.liferay.portal.kernel.dao.search.DisplayTerms;
import com.liferay.portal.kernel.util.ParamUtil;
import com.liferay.portal.util.PortalUtil;

import javax.portlet.RenderRequest;

/**
 * <a href="FeedDisplayTerms.java.html"><b><i>View Source</i></b></a>
 *
 * @author Raymond Augé
 *
 */
public class FeedDisplayTerms extends DisplayTerms {

	public static final String GROUP_ID = "groupId";

	public static final String FEED_ID = "searchFeedId";

	public static final String NAME = "name";

	public static final String DESCRIPTION = "description";

	public FeedDisplayTerms(RenderRequest req) {
		super(req);

		groupId = ParamUtil.getLong(
			req, GROUP_ID, PortalUtil.getPortletGroupId(req));
		feedId = ParamUtil.getString(req, FEED_ID);
		name = ParamUtil.getString(req, NAME);
		description = ParamUtil.getString(req, DESCRIPTION);
	}

	public long getGroupId() {
		return groupId;
	}

	public String getFeedId() {
		return feedId;
	}

	public String getName() {
		return name;
	}

	public String getDescription() {
		return description;
	}

	protected long groupId;
	protected String feedId;
	protected String name;
	protected String description;

}