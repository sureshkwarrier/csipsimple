/**
 * Copyright (C) 2010 Regis Montoya (aka r3gis - www.r3gis.fr)
 * This file is part of CSipSimple.
 *
 *  CSipSimple is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CSipSimple is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CSipSimple.  If not, see <http://www.gnu.org/licenses/>.
 */
package com.csipsimple.wizards.impl;

import android.text.InputType;

import com.csipsimple.R;
import com.csipsimple.api.SipProfile;
import com.csipsimple.utils.PreferencesWrapper;

public class OXO810 extends AlternateServerImplementation {
	
	@Override
	public void fillLayout(final SipProfile account) {
		super.fillLayout(account);
		

		accountUsername.setTitle(R.string.w_common_phone_number);
		accountUsername.setDialogTitle(R.string.w_common_phone_number);
		accountUsername.getEditText().setInputType(InputType.TYPE_CLASS_PHONE);
		

		accountServer.setText(account.getSipDomain().replace(":5059", ""));
	}
	

	
	public SipProfile buildAccount(SipProfile account) {
		account = super.buildAccount(account);
		
		account.transport = SipProfile.TRANSPORT_UDP;
		account.contact_rewrite_method = 1;
		return account;
	}

	@Override
	public void setDefaultParams(PreferencesWrapper prefs) {
		super.setDefaultParams(prefs);
	}

	@Override
	protected String getDomain() {
		return super.getDomain()+":5059";
	}

	@Override
	protected String getDefaultName() {
		return "OXO810";
	}
	
	@Override
	public boolean needRestart() {
		return false;
	}
	
}
